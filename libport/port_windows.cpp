#include <windows.h>

#include "port.h"

HThread thread_create(thread_callback proc, void* param)
{
	if (NULL == proc)
		return NULL;
	
	return CreateThread(NULL, //Choose default security
		0, //Default stack size
		(LPTHREAD_START_ROUTINE)proc,	//Routine to execute
		(LPVOID) param, //Thread parameter
		0, //Immediately run the thread
		NULL //Thread Id
		);
}

int thread_destroy(HThread thread)
{
	if (NULL == thread)
		return -1;
	
	WaitForSingleObject(thread, INFINITE);
	CloseHandle(thread);
	return 0;
}

int thread_suspend(HThread thread)
{
	if (NULL == thread)
		return -1;
	
	return SuspendThread(thread);
}

int thread_resume(HThread thread)
{
	if (NULL == thread)
		return -1;
	
	return ResumeThread(thread);
}

HMutex mutex_create()
{
	return CreateMutex(NULL, FALSE, NULL);
}

int mutex_destroy(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	WaitForSingleObject(mutex, INFINITE);
	return (0==CloseHandle(mutex));
}

int mutex_lock(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	WaitForSingleObject(mutex, INFINITE);
	return 0;
}

int mutex_lock_nowait(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	return WaitForSingleObject(mutex, 0);
}

int mutex_unlock(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	return (0==ReleaseMutex(mutex));
}

typedef struct _RWLock
{
	int count;
	HANDLE hRead;
	HANDLE hWrite;
}RWLock;

HRWLock CreateRWLock()
{
	RWLock *lock = (RWLock *)malloc(sizeof(RWLock));
	if (NULL == lock)
		return NULL;
	memset(lock, 0, sizeof(RWLock));
	
	lock->hRead = CreateSemaphore(NULL, 1, 1, NULL);
	if (NULL == lock->hRead)
		return NULL;

	lock->hWrite = CreateSemaphore(NULL, 1, 1, NULL);
	if (NULL == lock->hWrite)
		return NULL;

	return HRWLock(lock);
}

int DestroyRWLock(HRWLock hLock)
{
	if (NULL == hLock)
		return -1;

	RWLock *lock = (RWLock*)hLock;
	CloseHandle(lock->hRead);
	CloseHandle(lock->hWrite);
	free(lock);
	return 0;
}

int ReadLock(HRWLock hLock)
{
	if (NULL == hLock)
		return -1;

	RWLock *lock = (RWLock*)hLock;
	WaitForSingleObject(lock->hRead, INFINITE);
	if (0 == lock->count)
	{
		WaitForSingleObject(lock->hWrite, INFINITE);
	}
	lock->count++;
	ReleaseSemaphore(lock->hRead, 1, NULL);
	return 0;
}

int ReadUnlock(HRWLock hLock)
{
	if (NULL == hLock)
		return -1;

	RWLock *lock = (RWLock*)hLock;
	WaitForSingleObject(lock->hRead, INFINITE);
	lock->count--;
	if (0 == lock->count)
	{
		ReleaseSemaphore(lock->hWrite, 1, NULL);
	}
	ReleaseSemaphore(lock->hRead, 1, NULL);
	return 0;
}

int WriteLock(HRWLock hLock)
{
	if (NULL == hLock)
		return -1;
	
	RWLock *lock = (RWLock*)hLock;
	WaitForSingleObject(lock->hWrite, INFINITE);
	return 0;
}

int WriteUnlock(HRWLock hLock)
{
	if (NULL == hLock)
		return -1;
	
	RWLock *lock = (RWLock*)hLock;
	ReleaseSemaphore(lock->hWrite, 1, NULL);
	return 0;
}

int port_sleep(int ms)
{
	Sleep(ms);
	return 0;
}

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
#define SystemBasicInformation 0
#define SystemProcessorPerformanceInformation 8
#define TIME_COEFFICIENT 100
#define NANOSECOND100 (1000000000/TIME_COEFFICIENT)
#define POW24 1.6777216/*16777216/10000000*/
#define POW8 256

typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
typedef struct{ 
	DWORD dwUnknown1; 
	ULONG uKeMaximumIncrement; 
	ULONG uPageSize; 
	ULONG uMmNumberOfPhysicalPages; 
	ULONG uMmLowestPhysicalPage; 
	ULONG uMmHighestPhysicalPage; 
	ULONG uAllocationGranularity; 
	PVOID pLowestUserAddress; 
	PVOID pMmHighestUserAddress; 
	ULONG uKeActiveProcessors; 
	BYTE bKeNumberProcessors; 
	BYTE bUnknown2; 
	WORD wUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
_SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER Reserved1[2];
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

int GetCPUTotalFreq(int *MHz)
{
	int cpuNum=0, oneCpuFreq=0;
	unsigned   __int64 nFreq=0;
	PROCNTQSI NtQuerySystemInformation;
	SYSTEM_BASIC_INFORMATION SysBaseInfo;

	if (NULL == MHz)
		return -1;

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll"),"NtQuerySystemInformation"); 
	if (!NtQuerySystemInformation)
		return -1;

	if (0 != NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL))
		return -1;
	cpuNum = SysBaseInfo.bKeNumberProcessors;

	QueryPerformanceFrequency((LARGE_INTEGER   *)&nFreq);
	oneCpuFreq = nFreq/1000000;

	*MHz = oneCpuFreq * cpuNum;	
	return 0;
}

int GetCPUTime(unsigned int *runTime, unsigned int *allTime)
{
	PROCNTQSI NtQuerySystemInformation;
	SYSTEM_BASIC_INFORMATION SysBaseInfo;
	SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION *sysProcessorPerfInfo;

	if (NULL==runTime || NULL==allTime)
		return -1;

	*runTime = 0;
	*allTime = 0;
	
	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle("ntdll"),"NtQuerySystemInformation"); 
	if (!NtQuerySystemInformation)
		return -1;

	if (0 != NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL))
		return -1;

	sysProcessorPerfInfo = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[SysBaseInfo.bKeNumberProcessors];
	if (0 != NtQuerySystemInformation(SystemProcessorPerformanceInformation,sysProcessorPerfInfo,sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION)*SysBaseInfo.bKeNumberProcessors,NULL))
		return -1;

	for (int i=0; i<SysBaseInfo.bKeNumberProcessors; i++)
	{
		/*printf("%d kernel: %d %d, user: %d %d, idle: %d %d\r\n", i,
			sysProcessorPerfInfo[i].KernelTime.HighPart, sysProcessorPerfInfo[i].KernelTime.LowPart,
			sysProcessorPerfInfo[i].UserTime.HighPart, sysProcessorPerfInfo[i].UserTime.LowPart,
			sysProcessorPerfInfo[i].IdleTime.HighPart, sysProcessorPerfInfo[i].IdleTime.LowPart);*/
		*runTime += (unsigned int)(((double(sysProcessorPerfInfo[i].KernelTime.HighPart*POW8))*POW24
			+ (double(sysProcessorPerfInfo[i].KernelTime.LowPart))/NANOSECOND100)*TIME_COEFFICIENT);
		*runTime += (unsigned int)(((double(sysProcessorPerfInfo[i].UserTime.HighPart*POW8))*POW24
			+ (double(sysProcessorPerfInfo[i].UserTime.LowPart))/NANOSECOND100)*TIME_COEFFICIENT);
		*allTime += (unsigned int)(((double(sysProcessorPerfInfo[i].IdleTime.HighPart*POW8))*POW24
			+ ((double)(sysProcessorPerfInfo[i].IdleTime.LowPart))/NANOSECOND100)*TIME_COEFFICIENT) + *runTime;
	}
	
	delete []sysProcessorPerfInfo;
	return 0;
}

int GetProcessRunTime(int *time)
{
	FILETIME ftCreation, ftExit, ftKernel, ftUser;
	HANDLE hProcess = NULL;

	if (NULL == time)
		return -1;
	
	DuplicateHandle(GetCurrentProcess(), GetCurrentProcess(), GetCurrentProcess(),
  		&hProcess, 0, FALSE, DUPLICATE_SAME_ACCESS);

	GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser);

	*time = (int)(((double(ftKernel.dwHighDateTime*POW8))*POW24
		+ (double(ftKernel.dwLowDateTime))/NANOSECOND100
		+ (double(ftUser.dwHighDateTime*POW8))*POW24
		+ (double(ftUser.dwLowDateTime))/NANOSECOND100)*TIME_COEFFICIENT);
	
	return 0;
}


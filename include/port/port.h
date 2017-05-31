#ifndef _PORT_H_
#define _PORT_H_

/*thread*/
typedef void* HThread;
typedef void* (*thread_callback)(void*);
HThread thread_create(thread_callback proc, void* param);
int thread_destroy(HThread thread);
int thread_suspend(HThread thread);
int thread_resume(HThread thread);

/*mutex*/
typedef void* HMutex;
HMutex mutex_create();
int mutex_destroy(HMutex mutex);
int mutex_lock(HMutex mutex);
int mutex_lock_nowait(HMutex mutex);
int mutex_unlock(HMutex mutex);

typedef void* HRWLock;
HRWLock CreateRWLock();
int DestroyRWLock(HRWLock hLock);
int ReadLock(HRWLock hLock);
int ReadUnlock(HRWLock hLock);
int WriteLock(HRWLock hLock);
int WriteUnlock(HRWLock hLock);

/*sleep*/
int port_sleep(int ms);

/*cpu*/
int GetCPUTotalFreq(int *MHz);
int GetCPUTime(unsigned int *runTime, unsigned int *allTime);
int GetProcessRunTime(int *time);

#endif	//_PORT_H_


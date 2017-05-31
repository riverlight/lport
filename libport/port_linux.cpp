#ifndef WIN32

#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "port.h"

HThread thread_create(thread_callback proc, void* param)
{
	pthread_t *thread = NULL;
	
	thread = new pthread_t;
	if (NULL == proc)
		return NULL;
	
	if (0 == pthread_create(thread, NULL, proc, param))
	{
		return (HThread)thread;
	}
	else
	{
		delete thread;
		return NULL;
	}
}

int thread_destroy(HThread thread)
{
	if (NULL == thread)
		return -1;
	
	pthread_t thr = *((pthread_t*)thread);
	pthread_join(thr, NULL);
	
	delete (pthread_t *)thread;
	return 0;
}

int thread_suspend(HThread thread)
{
	return -1;
}

int thread_resume(HThread thread)
{
	return -1;
}

HMutex mutex_create()
{
	pthread_mutex_t *mutex = NULL;
	
	mutex = new pthread_mutex_t;
	if (NULL == mutex)
		return NULL;
	
	if (0 == pthread_mutex_init(mutex, NULL))
	{
		return (HMutex)mutex;
	}
	else
	{
		delete mutex;
		return NULL;
	}
}

int mutex_destroy(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	pthread_mutex_t *mu = (pthread_mutex_t*)mutex;
	pthread_mutex_destroy(mu);
	
	delete mu;
	return 0;
}

int mutex_lock(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	pthread_mutex_t *mu = (pthread_mutex_t*)mutex;
	return pthread_mutex_lock(mu);
}

int mutex_lock_nowait(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	pthread_mutex_t *mu = (pthread_mutex_t*)mutex;
	return pthread_mutex_trylock(mu);
}
int mutex_unlock(HMutex mutex)
{
	if (NULL == mutex)
		return -1;
	
	pthread_mutex_t *mu = (pthread_mutex_t*)mutex;
	return pthread_mutex_unlock(mu);
}

int port_sleep(int ms)
{
	usleep(ms*1000);
	return 0;
}

enum
{
	USER_TIME,
	NICE_TIME,
	SYSTEM_TIME,
	IDLE_TIME,
	IOWAIT_TIME,
	IRQ_TIME,
	SOFTIRQ_TIME,
	END_TIME
};

#define JIFFIES			100
#define FILE_BUF_SIZE	1024

int GetCPUTotalFreq(int *MHz)
{
	FILE *stat = NULL;
	int cpuNum=0, oneCpuFreq=0;
	int aPos=0, dotPos, size=0;
	char buf[FILE_BUF_SIZE];
	memset(buf, 0, FILE_BUF_SIZE);
	
	if (NULL == MHz)
	{
		return -1;
	}

	system("cat /proc/cpuinfo |grep name | cut -f2 -d: | uniq -c >cpu.txt");
	/*      4  Intel(R) Xeon(R) CPU           E5504  @ 2.00GHz*/
	stat = fopen("cpu.txt", "rb");
	if (NULL == stat)
	{
		return -1;
	}
	size = fread(buf, 1, FILE_BUF_SIZE, stat);
	fclose(stat);

	cpuNum = atoi(buf);

	for (dotPos=size-1; dotPos>0; dotPos--)
	{
		if ('.' == buf[dotPos])
		{
			break;
		}
	}
	if (0 == dotPos)
		return -1;
	oneCpuFreq = atoi(buf+dotPos+1)*10;

	for (aPos=dotPos-1; aPos>0; aPos--)
	{
		if ('@' == buf[aPos])
		{
			break;
		}
	}
	if (0 == aPos)
		return -1;
	oneCpuFreq += atoi(buf+aPos+1)*1000;

	*MHz = oneCpuFreq * cpuNum;

	return 0;
}

int GetCPUTime(unsigned int *runTime, unsigned int *allTime)
{
	FILE *stat = NULL;
	unsigned int time[END_TIME];
	int pos = 0, powTime = 0;
	char buf[FILE_BUF_SIZE];
	memset(buf, 0, FILE_BUF_SIZE);

	if (NULL==runTime || NULL==allTime)
		return -1;

	stat = fopen("/proc/stat", "rb");
	if (NULL == stat)
	{
		return -1;
	}

	fread(buf, 1, FILE_BUF_SIZE, stat);
	fclose(stat);

	pos = 5;
	*runTime = 0;
	*allTime = 0;

	for (int i=0; i<END_TIME; i++)
	{
		time[i] = atoi(buf+pos);
		powTime = log10(time[i]);
		pos += powTime+2;
		if (IDLE_TIME != i)
		{
			*runTime += time[i];
		}
		*allTime += time[i];
	}

	return 0;
}

/*2681 root      20   0  3260  924  796 S  0.0  0.2   0:01.00*/
int GetProcessRunTime(int *time)
{
	FILE *stat = NULL;
	int size = 0;
	int colonPos=0, spacePos=0, dotPos=0;;
	char cmd[100], pid[20], buf[FILE_BUF_SIZE];
	memset(cmd, 0, 100);
	memset(pid, 0, 20);
	memset(buf, 0, FILE_BUF_SIZE);

	if (NULL==time)
		return -1;

	sprintf(pid, "%d", getpid());
	sprintf(cmd, "top -p %s -n 1 >%s", pid, pid);
	system(cmd);

	stat = fopen(pid, "rb");
	if (NULL == stat)
	{
		return -1;
	}
	size = fread(buf, 1, FILE_BUF_SIZE, stat);
	fclose(stat);

	for (dotPos=size-1; dotPos>0; dotPos--)
	{
		if ('.' == buf[dotPos])
		{
			break;
		}
	}
	if (0 == dotPos)/*e.g. 3036:32*/
	{
		dotPos=size;
	}
	else/*e.g. 436:32.85*/
	{
		*time = atoi(buf+dotPos+1);
	}

	for (colonPos=dotPos-1; colonPos>0; colonPos--)
	{
		if (':' == buf[colonPos])
		{
			break;
		}
	}
	if (0 == colonPos)
		return -1;
	*time += atoi(buf+colonPos+1)*JIFFIES;
	
	for (spacePos=colonPos-1; spacePos>0; spacePos--)
	{
		if (0x20 == buf[spacePos])
		{
			break;
		}
	}
	if (0 == spacePos)
		return -1;
	*time += atoi(buf+spacePos+1)*JIFFIES*60;
	
	return 0;
}

#endif


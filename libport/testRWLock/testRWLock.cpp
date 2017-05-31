#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "port.h"

#define READ_THRERAD_NUM 5
#define THREAD_RUN_TIME 1000
#define THREAD_SLEEP_TIME 10

HRWLock hRWLock = NULL;

static void* read_thread_process(void *arg);
static void* write_thread_process(void *arg);

int main(int argc, char *argv[])
{
	int counter = 0;
	HThread hWriteThread = NULL;
	HThread hReadThread[READ_THRERAD_NUM];
	int id[READ_THRERAD_NUM];
	memset(hReadThread, 0, READ_THRERAD_NUM*sizeof(HThread));
	memset(id, 0, READ_THRERAD_NUM*sizeof(int));
	
	if (NULL != (hRWLock=CreateRWLock()))
	{
		printf("Create RWLock successfully\r\n\r\n");
	}
	else
	{		
		printf("Could not create RWLock\r\n\r\n");
		return -1;
	}

	if (NULL == (hWriteThread=thread_create(write_thread_process, NULL)))
	{
		printf("Could not create write thread\r\n\r\n");
		return -1;
	}

	for (counter=0; counter<READ_THRERAD_NUM; counter++)
	{
		id[counter] = counter+1;
		if (NULL == (hReadThread[counter] = thread_create(read_thread_process, id+counter)))
		{
			printf("Could not create %d thread\r\n\r\n", counter);
		}
		else
		{
			printf("Create %d thread successfully\r\n\r\n", counter);
		}
	}

	for (counter=0; counter<READ_THRERAD_NUM; counter++)
	{
		if (NULL != hReadThread[counter])
		{
			thread_destroy(hReadThread[counter]);
			printf("Destroy %d thread\r\n\r\n", counter);
		}
	}
	thread_destroy(hWriteThread);

#ifdef WIN32
	system("pause");
#endif

	DestroyRWLock(hRWLock);
	return 0;
}

static void* read_thread_process(void *arg)
{
	int threadId = *((int*)arg);
	for (int i=1; i<THREAD_RUN_TIME; i++)
	{
		ReadLock(hRWLock);
		printf("The %d thread have run %d times\r\n", threadId, i);
		ReadUnlock(hRWLock);
		port_sleep(THREAD_SLEEP_TIME);
	}
	return NULL;
}

static void* write_thread_process(void *arg)
{
	for (int i=1; i<THREAD_RUN_TIME; i++)
	{
		WriteLock(hRWLock);
		printf("The write thread have run %d times\r\n", i);
		WriteUnlock(hRWLock);
		port_sleep(THREAD_SLEEP_TIME);
	}
	return NULL;
}


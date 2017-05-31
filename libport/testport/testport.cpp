#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "port.h"

#define THRERA_NUM 10
#define THREAD_RUN_TIME 1000
#define THREAD_SLEEP_TIME 10

static void* thread_process(void *arg);

HMutex mutex = NULL;

int main(int argc, char *argv[])
{
	int counter = 0;
	HThread hThread[THRERA_NUM];
	int id[THRERA_NUM];
	memset(hThread, 0, THRERA_NUM*sizeof(HThread));
	memset(id, 0, THRERA_NUM*sizeof(int));

	if (NULL != (mutex = mutex_create()))
	{
		printf("Create mutex successfully\r\n\r\n");
	}
	else
	{
		printf("Could not create mutex\r\n\r\n");
		return -1;
	}
	
	for (counter=0; counter<THRERA_NUM; counter++)
	{
		id[counter] = counter+1;
		if (NULL == (hThread[counter] = thread_create(thread_process, id+counter)))
		{
			printf("Could not create %d thread\r\n\r\n", counter);
		}
		else
		{
			printf("Create %d thread successfully\r\n\r\n", counter);
		}
	}
	for (counter=0; counter<THRERA_NUM; counter++)
	{
		if (NULL != hThread[counter])
		{
			thread_destroy(hThread[counter]);
			printf("Destroy %d thread\r\n\r\n", counter);
		}
	}
	mutex_destroy(mutex);
#ifdef WIN32
	system("pause");
#endif
	return 0;
}

static void* thread_process(void *arg)
{
	int threadId = *((int*)arg);
	for (int i=1; i<THREAD_RUN_TIME; i++)
	{
		mutex_lock(mutex);
		printf("The %d thread have run %d times\r\n", threadId, i);
		mutex_unlock(mutex);
		port_sleep(THREAD_SLEEP_TIME);
	}
	return NULL;
}


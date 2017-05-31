#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "port.h"
#include "queue.h"

typedef struct Packet_s
{
	unsigned char *data;
	int size;
} Packet;

typedef struct Queue_s
{
	Packet *pPacket;
	int maxsize;
	int num;
	
	int count;
	int readpos, writepos;

	HMutex hMutex;
} Queue;

HQueue OpenQueue(int num, int bufsize)
{
	Queue *pQueue = NULL;

	if (num<=0 || bufsize<=0)
		return NULL;

	pQueue = (Queue *)malloc(sizeof(Queue));
	if (pQueue==NULL)
		return NULL;

	pQueue->maxsize = bufsize;
	pQueue->num = num;

	pQueue->pPacket = (Packet *)malloc(sizeof(Packet)*num);

	if (pQueue->pPacket==NULL)
		return NULL;

	for (int i=0; i<num; i++)
	{
		pQueue->pPacket[i].data = (unsigned char *)malloc(bufsize);
		if (pQueue->pPacket[i].data == NULL)
			return NULL;
		pQueue->pPacket[i].size = 0;
	}
	pQueue->hMutex = mutex_create();
	if (NULL == pQueue->hMutex)
		return NULL;

	pQueue->readpos = 0;
	pQueue->writepos = 0;
	pQueue->count = 0;

	return pQueue;
}

int CloseQueue(HQueue hQueue)
{
	Queue *pQueue = (Queue *)hQueue;

	if (NULL == hQueue)
		return QUEUE_ERROR_INPUT_ARGUMENT;

	pQueue->readpos = 0;
	pQueue->writepos = 0;
	pQueue->count = 0;

	mutex_destroy(pQueue->hMutex);

	for (int i=0; i< pQueue->num; i++)
	{
		free(pQueue->pPacket[i].data);
	}

	free(pQueue->pPacket);

	free(pQueue);

	return QUEUE_OK;
}

int GetQueueItem(HQueue hQueue, unsigned char *buf, int *psize)
{
	Queue *pQueue = (Queue *)hQueue;
	
	if (NULL==hQueue || NULL==buf)
		return QUEUE_ERROR_INPUT_ARGUMENT;

	mutex_lock(pQueue->hMutex);

	if (0 == pQueue->count)
	{
		mutex_unlock(pQueue->hMutex);
		return QUEUE_ERROR_EMPTY;
	}

	memcpy(buf, pQueue->pPacket[pQueue->readpos].data, pQueue->pPacket[pQueue->readpos].size);
	if (NULL != psize)
		*psize = pQueue->pPacket[pQueue->readpos].size;

	pQueue->count --;
	pQueue->readpos ++;
	if (pQueue->readpos == pQueue->num)
		pQueue->readpos = 0;
		
	mutex_unlock(pQueue->hMutex);

	return QUEUE_OK;
}

int PutQueueItem(HQueue hQueue, unsigned char *buf, int size)
{
	Queue *pQueue = (Queue *)hQueue;

	if (NULL==hQueue || NULL==buf || size<=0)
		return QUEUE_ERROR_INPUT_ARGUMENT;

	if (pQueue->maxsize < size)
		return QUEUE_ERROR_SIZE_TOO_LARGE;

	mutex_lock(pQueue->hMutex);

	if (pQueue->num == pQueue->count)
	{
		mutex_unlock(pQueue->hMutex);
		return QUEUE_ERROR_FULL;
	}

	memcpy(pQueue->pPacket[pQueue->writepos].data, buf, size);
	pQueue->pPacket[pQueue->writepos].size = size;

	pQueue->count ++;
	pQueue->writepos ++;
	if (pQueue->writepos == pQueue->num)
		pQueue->writepos = 0;

	mutex_unlock(pQueue->hMutex);

	return QUEUE_OK;
}

typedef struct MsgQueue_s
{
	HQueue hQueue;
	unsigned char *data;
	int size;

	HMutex hMutex;
} MsgQueue;

HMsgQueue OpenMsgQueue(int num, int bufsize)
{
	MsgQueue *pMsgQueue;

	if (num<=0 || bufsize<=0)
		return NULL;

	pMsgQueue = (MsgQueue *)malloc(sizeof(MsgQueue));
	if (pMsgQueue==NULL)
		return NULL;
	pMsgQueue->hQueue = OpenQueue(num, sizeof(Msg)+bufsize);
	if (pMsgQueue->hQueue==NULL)
		return NULL;
	pMsgQueue->hMutex = mutex_create();
	if (pMsgQueue->hMutex==NULL)
		return NULL;
	pMsgQueue->data = (unsigned char *)malloc(sizeof(Msg)+bufsize);
	if (pMsgQueue->data==NULL)
		return NULL;
	pMsgQueue->size = sizeof(Msg)+bufsize;

	return pMsgQueue;
}

int CloseMsgQueue(HMsgQueue hMsgQueue)
{
	MsgQueue *pMsgQueue = (MsgQueue *)hMsgQueue;
	
	if (NULL == hMsgQueue)
		return QUEUE_ERROR_INPUT_ARGUMENT;

	free(pMsgQueue->data);
	mutex_destroy(pMsgQueue->hMutex);
	CloseQueue(pMsgQueue->hQueue);
	free(pMsgQueue);

	return QUEUE_OK;
}

int GetMsgQueueItem(HMsgQueue hMsgQueue, Msg *pMsg)
{
	MsgQueue *pMsgQueue = (MsgQueue *)hMsgQueue;
	Msg tempmsg;
	int ret;

 	if (NULL==hMsgQueue || NULL==pMsg)
		return QUEUE_ERROR_INPUT_ARGUMENT;


	mutex_lock(pMsgQueue->hMutex);
	
	ret = GetQueueItem(pMsgQueue->hQueue, (unsigned char*)pMsgQueue->data, &pMsgQueue->size);
	if (ret != QUEUE_OK)
	{
		goto end;
	}
	
	memcpy(&tempmsg, pMsgQueue->data, sizeof(Msg));
	pMsg->msgtype = tempmsg.msgtype;
	pMsg->dataSize = tempmsg.dataSize;
	if (pMsg->dataSize!=0)
		memcpy(pMsg->pdata, pMsgQueue->data+sizeof(Msg), pMsg->dataSize);

end:	
	mutex_unlock(pMsgQueue->hMutex);

	return ret;
}

int PutMsgQueueItem(HMsgQueue hMsgQueue, Msg *pMsg)
{
	MsgQueue *pMsgQueue = (MsgQueue *)hMsgQueue;
	int len;
	int ret;
	
	if (NULL==hMsgQueue || NULL==pMsg)
		return QUEUE_ERROR_INPUT_ARGUMENT;

	mutex_lock(pMsgQueue->hMutex);
	memcpy(pMsgQueue->data, pMsg, sizeof(Msg));
	if (pMsg->dataSize!=0)
		memcpy(pMsgQueue->data+sizeof(Msg), pMsg->pdata, pMsg->dataSize);
	len = pMsg->dataSize + sizeof(Msg);
	
	ret = PutQueueItem(pMsgQueue->hQueue,  (unsigned char*)pMsgQueue->data, len);
	mutex_unlock(pMsgQueue->hMutex);

	return ret;
}


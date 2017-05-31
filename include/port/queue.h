#ifndef _QUEUE_H_
#define _QUEUE_H_

#define QUEUE_OK	0
#define QUEUE_ERROR_INPUT_ARGUMENT -1
#define QUEUE_ERROR_FULL -2
#define QUEUE_ERROR_EMPTY -3
#define QUEUE_ERROR_MEMORY_ALLOCATE -4
#define QUEUE_ERROR_MUTEX -5
#define QUEUE_ERROR_SIZE_TOO_LARGE -6

typedef void* HQueue;

HQueue OpenQueue(int num, int bufsize);
int CloseQueue(HQueue hQueue);

int GetQueueItem(HQueue hQueue, unsigned char *buf, int *psize);
int PutQueueItem(HQueue hQueue, unsigned char *buf, int size);

typedef struct Msg_s
{
	int msgtype;
	int dataSize;
	unsigned char *pdata;
} Msg;

typedef void* HMsgQueue;
HMsgQueue OpenMsgQueue(int num, int bufsize);
int CloseMsgQueue(HMsgQueue hMsgQueue);
int GetMsgQueueItem(HMsgQueue hMsgQueue, Msg *pMsg);
int PutMsgQueueItem(HMsgQueue hMsgQueue, Msg *pMsg);

#endif // _QUEUE_H_


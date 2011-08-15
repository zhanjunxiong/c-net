
#ifndef _EVENT_MSGQUEUE_H_
#define _EVENT_MSGQUEUE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "dataType.h"

#include <pthread.h>

struct aeEventLoop;

typedef struct eventMsgqueue {
   int pushFd;
   int popFd;
   int unlockBetweenCallbacks;
   int mask;

   aeEventLoop *el;

   pthread_mutex_t lock;

   void (*callback)(void *, void *);
   void *cbarg;
   struct circqueue *queue;
}eventMsgqueue;

struct eventMsgqueue *initMsgqueue(struct aeEventLoop* eventLoop, unsigned int, void (*)(void *, void *), void *);
int pushMsgqueue(struct eventMsgqueue *, void *);
unsigned int lengthMsgqueue(struct eventMsgqueue *);
void destroyMsgqueue(struct eventMsgqueue*);

#ifdef __cplusplus
}
#endif

#endif


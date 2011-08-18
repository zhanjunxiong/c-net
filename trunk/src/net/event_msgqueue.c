
#include "ae.h"
#include "err.h"
#include "event_msgqueue.h"
#include "mylog.h"
#include "zmalloc.h"

typedef struct circqueue {
   unsigned int head;
   unsigned int tail;
   unsigned int count;
   unsigned int maxEntries;
   unsigned int arrayElements;
   void **entries;
}circqueue;

#define DEFAULT_UNBOUNDED_QUEUE_SIZE 1024

static unsigned int nextpow2(unsigned int num) {
    --num;
    num |= num >> 1;
    num |= num >> 2;
    num |= num >> 4;
    num |= num >> 8;
    num |= num >> 16;
    return ++num;
}

#define getLengthCircqueue(q) ((q)->count)
#define isEmptyCircqueue(q) (!getLengthCircqueue(q))
#define isFullcircqueue(q) ((q)->count == (q)->arrayElements)

static struct circqueue *initCircqueue(unsigned int size) {
   struct circqueue *cq = NULL;

   if (!(cq = zcalloc(sizeof(struct circqueue)))) {
      return(NULL);
   }

   cq->maxEntries = size;
   if (!size || !(cq->arrayElements = nextpow2(size))) {
      cq->arrayElements = DEFAULT_UNBOUNDED_QUEUE_SIZE;
   }

   cq->entries = zcalloc(sizeof(void *) * cq->arrayElements);
   if (!cq->entries) {
      zfree(cq);
      return(NULL);
   }

   return(cq);
}

static void destroyCircqueue(struct circqueue *cq) {
   zfree(cq->entries);
   zfree(cq);
}

static int growCircqueue(struct circqueue *cq) {
   void **newents;
   unsigned int newsize = cq->arrayElements << 1;
   unsigned int headchunklen = 0, tailchunklen = 0;
   
   if (!(newents = zcalloc(sizeof(void *) * newsize))) {
      return(-1);
   }

   if (cq->head < cq->tail) {
      headchunklen = cq->tail - cq->head;
   }
   else {
      headchunklen = cq->arrayElements - cq->head;
      tailchunklen = cq->tail;
   }

   memcpy(newents, &cq->entries[cq->head], sizeof(void *) * headchunklen);
   if (tailchunklen) {
      memcpy(&newents[headchunklen], cq->entries, sizeof(void *) * tailchunklen);
   }

   cq->head = 0;
   cq->tail = headchunklen + tailchunklen;
   cq->arrayElements = newsize;

   zfree(cq->entries);
   cq->entries = newents;

   return(0);
}

static int pushTailCircqueue(struct circqueue *cq, void *elem) {
   if (cq->maxEntries) {
      if (cq->count == cq->maxEntries) {
         return(-1);
      }
   } else if (isFullcircqueue(cq) && growCircqueue(cq) != 0) {
      return(-1);
   }

   cq->count++;
   cq->entries[cq->tail++] = elem;
   cq->tail &= cq->arrayElements - 1;

   return(0);
}

static void *popHeadCircqueue(struct circqueue *cq) {
   void *data = NULL;

   if (!cq->count) {
      return(NULL);
   }

   cq->count--;
   data = cq->entries[cq->head++];
   cq->head &= cq->arrayElements - 1;

   return(data);
}

static void popMsgqueue(aeEventLoop* el, int fd, void *arg, int mask) {

   struct eventMsgqueue *msgq = arg;
   char buf[64];

   recv(fd, buf, sizeof(buf),0);

   pthread_mutex_lock(&msgq->lock);
   while(!isEmptyCircqueue(msgq->queue)) {
      void *qdata = NULL;

      qdata = popHeadCircqueue(msgq->queue);

      if (msgq->unlockBetweenCallbacks) {
         pthread_mutex_unlock(&msgq->lock);
      }

      msgq->callback(qdata, msgq->cbarg);

      if (msgq->unlockBetweenCallbacks) {
         pthread_mutex_lock(&msgq->lock);
      }
   }
   pthread_mutex_unlock(&msgq->lock);
}

struct eventMsgqueue *initMsgqueue(aeEventLoop* eventLoop, unsigned int maxSize, void (*callback)(void *, void *), void *cbarg) {
   if (!eventLoop) {
	   return NULL;
   }

   struct eventMsgqueue *msgq;
   struct circqueue *cq;
   int fds[2];

   if (!(cq = initCircqueue(maxSize)))
      return(NULL);

   if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
      destroyCircqueue(cq);
      return(NULL);
   }

   if (!(msgq = zmalloc(sizeof(struct eventMsgqueue)))) {
      destroyCircqueue(cq);
      close(fds[0]);
      close(fds[1]);
      return(NULL);
   }

   msgq->pushFd = fds[0];
   msgq->popFd = fds[1];
   msgq->queue = cq;
   msgq->callback = callback;
   msgq->cbarg = cbarg;
   pthread_mutex_init(&msgq->lock, NULL);

   msgq->el = eventLoop;
   msgq->unlockBetweenCallbacks = 1;
   
   aeCreateFileEvent(msgq->el, msgq->popFd, AE_READABLE, popMsgqueue, msgq);
   return(msgq);
}

void destroyMsgqueue(struct eventMsgqueue *msgq)
{
   //for( ; lengthMsgqueue(msgq) > 0; ) {
	//   sleep( 1 );
   //}

   aeDeleteFileEvent(msgq->el, msgq->popFd, AE_READABLE);
   pthread_mutex_destroy(&msgq->lock);
   destroyCircqueue(msgq->queue);
   close(msgq->pushFd);
   close(msgq->popFd);

   zfree(msgq);
}

int pushMsgqueue(struct eventMsgqueue *msgq, void *msg) {
   const char buf[1] = { 0 };
   int r = 0;

   pthread_mutex_lock(&msgq->lock);
   if ((r = pushTailCircqueue(msgq->queue, msg)) == 0) {
      if (getLengthCircqueue(msgq->queue) == 1) {
         send(msgq->pushFd, buf, 1,0);
      }
   }
   pthread_mutex_unlock(&msgq->lock);

   return(r);
}

unsigned int lengthMsgqueue(struct eventMsgqueue *msgq) {
   unsigned int len = 0;

   pthread_mutex_lock(&msgq->lock);
   len = getLengthCircqueue(msgq->queue);
   pthread_mutex_unlock(&msgq->lock);

   return(len);
}


/*
 * eventLoopThreadPool.c
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#include "err.h"
#include "eventLoopThread.h"
#include "eventLoopThreadPool.h"
#include "mylog.h"

struct eventLoopThreadPool* initEventLoopThreadPool(struct eventLoop* baseLoop, uint32 threadNum) {
	struct eventLoopThreadPool* pool = (struct eventLoopThreadPool*)zcalloc(sizeof(struct eventLoopThreadPool));
	pool->threadNum = threadNum;
	pool->thread = (struct eventLoopThread**)zcalloc(sizeof(struct eventLoopThreadPool*) * threadNum);
	uint32 i;
	for (i = 0; i < pool->threadNum; i++) {
		pool->thread[i] = initEventLoopThread();
	}
	pool->state = START;
	pool->baseLoop = baseLoop;
	return pool;
}

void destroyEventLoopThreadPool(struct eventLoopThreadPool* threadPool) {
	myAssert(threadPool != NULL);
	uint32 i;
	for (i = 0; i < threadPool->threadNum; i++) {
		destroyEventLoopThread(threadPool->thread[i]);
	}
	zfree(threadPool->thread);
	zfree(threadPool);
}

void startEventLoopThreadPool(struct eventLoopThreadPool* threadPool) {
	myAssert(threadPool != NULL);
	uint32 i;
	for (i = 0; i < threadPool->threadNum; i++) {
		startEventLoopThread(threadPool->thread[i]);
	}
	threadPool->state = RUNNING;
}

void stopEventLoopThreadPool(struct eventLoopThreadPool* threadPool) {
	myAssert(threadPool != NULL);
	uint32 i;
	for (i = 0; i < threadPool->threadNum; i++) {
		stopEventLoopThread(threadPool->thread[i]);
	}
	threadPool->state = STOP;
}

static uint32 nextId = 0;
struct eventLoop* getNextEventLoop(struct eventLoopThreadPool* threadPool) {
	myAssert(threadPool != NULL);
	if (threadPool->threadNum == 0) {
		return threadPool->baseLoop;
	}
	mylog(LOG_INFO, "next eventLoop nextId: %d", nextId);
	struct eventLoopThread* thread = threadPool->thread[nextId];
	myAssert(thread != NULL);

	struct eventLoop* loop = thread->eventLoop;
	if (++nextId >= threadPool->threadNum) {
		nextId = 0;
	}
	return loop;
}

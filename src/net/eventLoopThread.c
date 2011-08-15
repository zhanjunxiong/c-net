/*
 * eventLoopThread.c
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#include "err.h"
#include "eventLoop.h"
#include "eventLoopThread.h"

void* eventThread(void* args) {
	struct eventLoopThread* t = (struct eventLoopThread*) args;
	if (!t) {
		return NULL;
	}

	pthread_mutex_lock(&t->mutex);
	t->eventLoop = initEventLoop(gettid());
    pthread_cond_signal(&t->cond);
	pthread_mutex_unlock(&t->mutex);
	loopEventLoop(t->eventLoop);
	return NULL;
}

struct eventLoopThread* initEventLoopThread() {
	struct eventLoopThread* thread = (struct eventLoopThread* )zcalloc(sizeof(struct eventLoopThread));
	pthread_mutex_init(&thread->mutex, NULL);
	 pthread_cond_init(&thread->cond, NULL);
	return thread;
}

void destroyEventLoopThread(struct eventLoopThread* thread) {
	myAssert(thread != NULL);
	if (thread->eventLoop) {
		destroyEventLoop(thread->eventLoop);
	}
	pthread_mutex_destroy(&thread->mutex);
	pthread_cond_destroy(&thread->cond);
	zfree(thread);
}

int32 startEventLoopThread(struct eventLoopThread* thread) {
	myAssert(thread != NULL);
	pthread_create( &thread->pid, NULL, eventThread, (void* )thread);

	pthread_mutex_lock(&thread->mutex);
	while(thread->eventLoop == NULL) {
		pthread_cond_wait(&thread->cond, &thread->mutex);
	}
	pthread_mutex_unlock(&thread->mutex);
	startEventLoop(thread->eventLoop);
	return 0;
}

void stopEventLoopThread(struct eventLoopThread* thread) {
	myAssert(thread != NULL);
	stopEventLoop(thread->eventLoop);
	pthread_t tid = thread->pid;
	pthread_join(tid, NULL);
}


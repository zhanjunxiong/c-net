/*
 * eventLoopThread.h
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#ifndef EVENTLOOPTHREAD_H_
#define EVENTLOOPTHREAD_H_

#include "dataType.h"

#include <pthread.h>

#define THREAD_START			0
#define THREAD_RUNNING		1
#define	THREAD_STOP				2

struct eventLoop;

typedef struct eventLoopThread {
	struct eventLoop* eventLoop;
	pthread_t pid;
	uint32 state;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
} eventLoopThread;

struct eventLoopThread* initEventLoopThread();
void destroyEventLoopThread(struct eventLoopThread* thread);

int32 startEventLoopThread(struct eventLoopThread* thread);
void stopEventLoopThread(struct eventLoopThread* thread);

#endif /* EVENTLOOPTHREAD_H_ */

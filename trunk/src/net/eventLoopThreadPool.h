/*
 * eventLoopThreadPool.h
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#ifndef EVENTLOOPTHREADPOOL_H_
#define EVENTLOOPTHREADPOOL_H_

#include "dataType.h"

#define	START		0
#define	RUNNING	1
#define	STOP			2

struct eventLoop;
struct eventLoopThread;

typedef struct eventLoopThreadPool {
	struct eventLoopThread** thread;
	uint32 threadNum;
	uint32	state;
	struct eventLoop* baseLoop;
} eventLoopThreadPool;

struct eventLoopThreadPool* initEventLoopThreadPool(struct eventLoop* baseLoop, uint32 threadNum);
void destroyEventLoopThreadPool(struct eventLoopThreadPool* threadPool);

void startEventLoopThreadPool(struct eventLoopThreadPool* threadPool);
void stopEventLoopThreadPool(struct eventLoopThreadPool* threadPool);

struct eventLoop* getNextEventLoop(struct eventLoopThreadPool* threadPool);

#endif /* EVENTLOOPTHREADPOOL_H_ */

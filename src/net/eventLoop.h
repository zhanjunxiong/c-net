/*
 * eventLoop.h
 *
 *  Created on: 2011-8-6
 *      Author:
 */

#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#ifdef __cpluscplus
extern "C" {
#endif

#include "dataType.h"

#define EVENTLOOP_START			0
#define	EVENTLOOP_RUN			1
#define	EVENTLOOP_STOPING	2
#define	EVENTLOOP_STOP			3

struct aeEventLoop;
struct buffer;
struct eventMsgqueue;
struct session;

typedef struct eventLoop {
	pid_t	pid; //该eventloop 所在的线程ID
	uint32 state;
	struct aeEventLoop* loop;
	struct eventMsgqueue* msgQueue;
} eventLoop;

struct eventLoop* initEventLoop(pid_t pid);
void destroyEventLoop(struct eventLoop* loop);

void startEventLoop(struct eventLoop* loop);
void stopEventLoop(struct eventLoop* loop);

int32 updateSessionEvent(struct eventLoop* loop, struct session* session);
int32 removeSessionEvent(struct eventLoop* loop, struct session* session);
int32 sendSessionEvent(struct eventLoop* loop, struct session* session, struct buffer* sendBuf);

void loopEventLoop(struct eventLoop* loop);

#ifdef __cpluscplus
}
#endif

#endif /* EVENTLOOP_H_ */

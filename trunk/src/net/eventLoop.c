/*
 * eventLoop.c
 *
 *  Created on: 2011-8-6
 *      Author:
 */

#include "ae.h"
#include "err.h"
#include "event_msgqueue.h"
#include "eventLoop.h"
#include "session.h"
#include "zmalloc.h"
#include "mylog.h"
#include "tcpClient.h"

#define	ADDSESSION			0
#define	REMOVESESSION		1
#define  SENDSESSION			2
#define	CONNECT					3

typedef struct msgEvent {
	uint32 eventType;
	struct session* session;
	onTimeCallBack* timeCallBack;
	void* arg;
} msgEvent;

void onWriteSession(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);
void onReadSession(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask);

 void eventCallBack(void* arg1, void* arg2) {
	 struct msgEvent* event = (struct msgEvent*)arg1;
	 struct eventLoop* loop = (struct eventLoop* )arg2;
	 myAssert(event != NULL && loop != NULL);

	 struct session* session = event->session;
	 myAssert(session != NULL);

	 if (event->eventType == REMOVESESSION) {
		 if (session->mask & SESSION_WRITABLE) {
			 aeDeleteFileEvent(loop->loop, session->cfd, AE_WRITABLE);
		 }

		 if (session->mask & SESSION_READABLE) {
			 aeDeleteFileEvent(loop->loop, session->cfd, AE_READABLE);
		 }

		 destroySession(session);
	 } else if (event->eventType == ADDSESSION) {
		 session->loop = loop;
		if (session->mask & SESSION_WRITABLE) {
			aeCreateFileEvent(loop->loop, session->cfd, AE_WRITABLE, onWriteSession, session);
		}
		if (session->mask & SESSION_READABLE) {
			aeCreateFileEvent(loop->loop, session->cfd, AE_READABLE, onReadSession, session);
		}
	 } else if (event->eventType == SENDSESSION) {
		 struct buffer* sendBuf = (struct buffer* )event->arg;
		 writeSession(session, sendBuf);
	 } else if (event->eventType == CONNECT) {
		 struct tcpClient* tcpClient = (struct tcpClient*)event->arg;
		 tcpClient->state = tcpClient_kConnecting;
		 startConnectInLoop(tcpClient);
	 }
	 zfree(event);
 }

void closeWriteStateSession(struct session* session) {
	myAssert(session != NULL);
 	struct eventLoop* loop = session->loop;

 	aeDeleteFileEvent(loop->loop, session->cfd, AE_WRITABLE);
 	retrieveAllBuffer(session->sendBuffer);
 	if (session->mask == SESSION_NONE) return;
 	session->mask = session->mask & (~SESSION_WRITABLE);
 }


void onWriteSession(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
 	struct session* session = (struct session*) clientData;
 	if (!session) {
 		return;
 	}
 	int sendComplete = 0;
 	while(1) {
 		int sendLen = 0;
 		sendLen = send(session->cfd, (void*)getReadIndexBuffer(session->sendBuffer), getReadableBytesBuffer(session->sendBuffer), 0);
 		if (sendLen > 0) {
 			moveReadIndexBuffer(session->sendBuffer, sendLen);
 			if (getReadableBytesBuffer(session->sendBuffer) <= 0) {
 				sendComplete = 1;
 				break;
 			}
 		} else if (sendLen == -1) {
 			if (errno == EAGAIN) {
 				break;
 			} else {
 				break;
 			}
 		} else {
 			break;
 		}
 	}

 	if (sendComplete == 1) {
 		closeWriteStateSession(session);
 		if (session->callBackHandle) {
 			if (session->callBackHandle->writeCompleteCallBack) {
 				session->callBackHandle->writeCompleteCallBack(session);
 			}
 		}
 	}
}

void onReadSession(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
	struct session* session = (struct session* )clientData;
	if (!session) {
		return;
	}

	//if (session->state != kConnected || session->state == kConnecting) {
	//	return;
	//}

	if (session->state == kConnecting) {
		if (session->callBackHandle) {
			if (session->callBackHandle->connectionCallBack) {
				session->callBackHandle->connectionCallBack(session, session->privateData);
			}
		}
	}
	mylog(LOG_INFO, "session state: %d", session->state);
	int32 len;
	len = readSession(session);
	if (len <= 0) {
		if (session->callBackHandle) {
			if (session->callBackHandle->disConnectionCallBack) {
				session->callBackHandle->disConnectionCallBack(session, session->privateData);
			}
		}

		removeSessionEvent(session->loop, session);
	} else if (len > 0){
		int ret;
		if (session->callBackHandle) {
			if (session->callBackHandle->dataCallBack) {
				ret = session->callBackHandle->dataCallBack(session, session->privateData);
			}
		}

		if (ret == SHUTDOWN || ret == CLOSE) {
			if (session->callBackHandle) {
				if (session->callBackHandle->disConnectionCallBack) {
					session->callBackHandle->disConnectionCallBack(session, session->privateData);
				}
			}

			removeSessionEvent(session->loop, session);
		}
	}
}

int eventLoopCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
	struct eventLoop* loop = (struct eventLoop*) clientData;
	if (!loop) {
		return AE_NOMORE;
	}

	if (loop->state == EVENTLOOP_STOP) {
		if (loop->loop) {
			aeStop(loop->loop);
			loop->state = EVENTLOOP_STOPING;
			return AE_NOMORE;
		}
	}
	return 100; /* every 100ms */
}

#define MSGQUEUESIZE	128
struct eventLoop* initEventLoop(pid_t pid) {
	struct eventLoop* loop = (struct eventLoop*)zcalloc(sizeof(struct eventLoop));
	loop->loop = aeCreateEventLoop();
	myAssert(loop->loop != NULL);
	loop->pid = pid;
	loop->state = EVENTLOOP_START;
	loop->msgQueue = initMsgqueue(loop->loop,
																	MSGQUEUESIZE,
																	eventCallBack,
																	(void*)loop);

	myAssert(loop->msgQueue != NULL);
	aeCreateTimeEvent(loop->loop,
										1,
										eventLoopCron,
										loop,
										NULL);
	return loop;
}

void destroyEventLoop(struct eventLoop* loop) {
	myAssert(loop);
	destroyMsgqueue(loop->msgQueue);
	aeDeleteEventLoop(loop->loop);
	zfree(loop);
}

void startEventLoop(struct eventLoop* loop) {
	myAssert(loop);
	loop->state = EVENTLOOP_RUN;
}

void stopEventLoop(struct eventLoop* loop) {
	myAssert(loop);
	loop->state = EVENTLOOP_STOP;
}

int32 updateSessionEvent(struct eventLoop* loop, struct session* session) {
	myAssert(loop != NULL && session != NULL);
	//if (session->state != kConnected) {
	//	return -1;
	//}

	if (loop->state != EVENTLOOP_RUN) {
		mylog(LOG_INFO, "LOOP NOT IN EVENTLOOP");
		return -1;
	}

	if (gettid() == loop->pid) {
		session->loop = loop;
		if (session->mask & SESSION_WRITABLE) {
			aeCreateFileEvent(loop->loop, session->cfd, AE_WRITABLE, onWriteSession, session);
		}
		if (session->mask & SESSION_READABLE) {
			aeCreateFileEvent(loop->loop, session->cfd, AE_READABLE, onReadSession, session);
		}
	} else {
		struct msgEvent* event = (struct msgEvent*)zmalloc(sizeof(struct msgEvent));
		event->session = session;
		event->eventType = ADDSESSION;
		pushMsgqueue(loop->msgQueue, event);
	}
	return 0;
}

int32 removeSessionEvent(struct eventLoop* loop, struct session* session) {
	myAssert(loop != NULL);
	myAssert(session != NULL);
	if (session->state != kConnected) {
		return -1;
	}

	if (gettid() == loop->pid) {
		if (session->mask & SESSION_WRITABLE) {
			aeDeleteFileEvent(loop->loop, session->cfd, AE_WRITABLE);
		}
		if (session->mask & SESSION_READABLE) {
			aeDeleteFileEvent(loop->loop, session->cfd, AE_READABLE);
		}
		destroySession(session);
	} else {
		struct msgEvent* event = (struct msgEvent*)zmalloc(sizeof(struct msgEvent));
		event->session = session;
		event->eventType = REMOVESESSION;
		session->state = kDisconnecting;
		pushMsgqueue(loop->msgQueue, event);
	}
	return 0;
}

int32 sendSessionEvent(struct eventLoop* loop, struct session* session, struct buffer* sendBuf) {
	myAssert(loop != NULL && session != NULL);
	if (session->state != kConnected) {
		return -1;
	}

	if (loop->state != EVENTLOOP_RUN) {
		mylog(LOG_INFO, "LOOP NOT IN EVENTLOOP");
		return -1;
	}

	if (gettid() == loop->pid) {
		writeSession(session, sendBuf);
	} else {
		struct msgEvent* event = (struct msgEvent*)zmalloc(sizeof(struct msgEvent));
		event->session = session;
		event->arg = sendBuf;
		event->eventType = SENDSESSION;
		pushMsgqueue(loop->msgQueue, event);
	}
	return 0;
}

int32 connectEvent(struct eventLoop* loop, struct tcpClient* tcpClient) {
	myAssert(loop != NULL && tcpClient != NULL);
	if (loop->state != EVENTLOOP_RUN) {
		return -1;
	}

	if (loop->state != EVENTLOOP_RUN) {
		mylog(LOG_INFO, "LOOP NOT IN EVENTLOOP");
		return -1;
	}

	if (gettid() == loop->pid) {
		tcpClient->state = tcpClient_kConnecting;
		startConnectInLoop(tcpClient);
	} else {
		struct msgEvent* event = (struct msgEvent*)zmalloc(sizeof(struct msgEvent));
		event->arg = tcpClient;
		event->eventType = CONNECT;
		pushMsgqueue(loop->msgQueue, event);
	}
}

int interTimeProc(struct aeEventLoop *eventLoop, long long id, void *clientData) {
	myAssert(eventLoop != NULL && clientData != NULL);
	struct msgEvent* event = (struct msgEvent*)clientData;
	event->timeCallBack(event->arg);
	return AE_NOMORE;
}

void interEventFinalizerProc(struct aeEventLoop *eventLoop, void *clientData) {
	myAssert(eventLoop != NULL);
	if (clientData) {
		zfree(clientData);
	}
}

int64 createTimeEvent(struct eventLoop* loop, int64 milliseconds, onTimeCallBack* callBack, void* arg) {
	myAssert(loop != NULL);
	if (loop->state != EVENTLOOP_RUN) {
		return -1;
	}
	if (gettid() == loop->pid) {
		struct msgEvent* clientData = (struct msgEvent*)zmalloc(sizeof(struct msgEvent));
		clientData->arg = arg;
		clientData->timeCallBack = callBack;
		aeCreateTimeEvent(loop->loop, milliseconds, interTimeProc, clientData, interEventFinalizerProc);
	} else {

	}
}

int32 deleteTimeEvent(struct eventLoop *loop, int64 id) {

}

void loopEventLoop(struct eventLoop* loop) {
	myAssert(loop != NULL);
	aeMain(loop->loop);
}

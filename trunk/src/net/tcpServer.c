/*
 * tcpServer.c
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#include "ae.h"
#include "anet.h"
#include "callback.h"
#include "err.h"
#include "eventLoop.h"
#include "eventLoopThreadPool.h"
#include "fastArray.h"
#include "tcpServer.h"
#include "session.h"
#include "zmalloc.h"
#include "mylog.h"

void removeSession(struct tcpServer* server, struct session* session) {
	myAssert(server != NULL && session != NULL);
	pthread_mutex_lock(&server->sessionArrayLock);
	eraseFastArray(server->sessionArray, (struct arrayItem* )session);
	pthread_mutex_unlock(&server->sessionArrayLock);
}

void tcpServerDisConnection(struct session* session, void* privateData) {
	myAssert(session != NULL);
	struct tcpServer* server = (struct tcpServer*)session->server;
	if (server->callBack) {
		if (server->callBack->disConnectionCallBack) {
			server->callBack->disConnectionCallBack(session, privateData);
		}
	}

	removeSession(server, session);
}

struct session* getSession(struct tcpServer* server) {
	myAssert(server != NULL);

	struct session* session = initSession();
	myAssert(session != NULL);

	char neterr[256];
	int cfd = anetTcpAccept(neterr, server->listenfd, session->cip,  &session->cport);
    if (cfd == AE_ERR) {
    	mylog(LOG_INFO, "accept error!!!");
    	destroySession(session);
        return NULL;
    }
    anetNonBlock(NULL,  cfd);
    anetTcpNoDelay(NULL, cfd);
	session->server = server;
	session->cfd = cfd;
	session->state = kConnected;
	session->callBackHandle->connectionCallBack = server->callBack->connectionCallBack;
	session->callBackHandle->dataCallBack = server->callBack->dataCallBack;
	session->callBackHandle->disConnectionCallBack = tcpServerDisConnection;
	pthread_mutex_lock(&server->sessionArrayLock);
	pushBackFastArray(server->sessionArray, (struct arrayItem* )session);
	pthread_mutex_unlock(&server->sessionArrayLock);
	return session;
}

static void onAccept(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
	struct tcpServer* server = (struct tcpServer*) clientData;
	if (!server) {
		return;
	}

	struct session* session = getSession(server);
	myAssert(session != NULL);
	struct eventLoop* loop = getNextEventLoop(server->threadPool);
	myAssert(loop != NULL);
	session->loop = loop;
	int32 ret = updateSessionEvent(loop, session);
	mylog(LOG_INFO, "onAccept ret: %d", ret);
}

#define INIT_SESSIONARRAY_SIZE	128
struct tcpServer* initTcpServer(struct eventLoop* baseLoop, uint32 threadNum, const char* listenAddr, uint32 listenPort) {
	struct tcpServer* server = (struct tcpServer*)zcalloc(sizeof(struct tcpServer));
	server->baseLoop = baseLoop;
	server->listenAddr = zstrdup(listenAddr);
	server->listenport = listenPort;
	server->threadPool = initEventLoopThreadPool(baseLoop, threadNum);
	server->sessionArray = initFastArray(INIT_SESSIONARRAY_SIZE);
	server->listenfd = -1;
	server->callBack = zcalloc(sizeof(struct ICallBack));
	pthread_mutex_init(&server->sessionArrayLock, NULL);
	return server;
}

void destroyTcpServer(struct tcpServer* server) {
	myAssert(server != NULL);
	pthread_mutex_destroy(&server->sessionArrayLock);
	destroyEventLoopThreadPool(server->threadPool);
	destroyFastArray(server->sessionArray);
	if (server->listenAddr) {
		zfree(server->listenAddr);
	}
	if (server->callBack) {
		zfree(server->callBack);
	}
	zfree(server);
}

uint32 startTcpServer(struct tcpServer* server) {
	myAssert(server != NULL && server->baseLoop != NULL);
	startEventLoopThreadPool(server->threadPool);
	server->listenfd = anetTcpServer(NULL, server->listenport, server->listenAddr);
	myAssert(server->listenfd != ANET_ERR);
    anetNonBlock(NULL,  server->listenfd);
    anetTcpNoDelay(NULL, server->listenfd);
    struct aeEventLoop* eventLoop = server->baseLoop->loop;
	aeCreateFileEvent(eventLoop,
			server->listenfd,
			AE_READABLE,
			onAccept,
			(void*)server);
	return 0;
}

void stopTcpServer(struct tcpServer* server) {
	myAssert(server != NULL);
	if (server->listenfd) {
		close(server->listenfd);
	}
	pthread_mutex_lock(&server->sessionArrayLock);
	while(1) {
		uint32 tail = getTailFastArray(server->sessionArray);
		mylog(LOG_INFO, "TAIL: %d", tail);
		if (tail < 1) {
			mylog(LOG_INFO, "break TAIL: %d", tail);
			break;
		}
		struct arrayItem* arrayItem = (struct arrayItem*)getArrayItemFastArray(server->sessionArray, tail - 1);
		eraseFastArray(server->sessionArray, arrayItem);
		struct session* session = (struct session*)arrayItem;
		removeSessionEvent(session->loop, session);
	}
	pthread_mutex_unlock(&server->sessionArrayLock);
	stopEventLoopThreadPool(server->threadPool);
}

/*
 * tcpClient.c
 *
 *  Created on: 2011-8-17
 *      Author:
 */
#include "ae.h"
#include "anet.h"
#include "callback.h"
#include "err.h"
#include "eventLoop.h"
#include "tcpClient.h"
#include "session.h"
#include "mylog.h"
#include "zmalloc.h"

int32 startConnectInLoop(void* tcpClient);
void realConnect(struct tcpClient* tcpClient, fd_t fd);
static const int kMaxRetryDelayMs = 30*1000;

void onReConnect(struct tcpClient* tcpClient) {
	myAssert(tcpClient != NULL);
	if (tcpClient->retry == 1) {
		tcpClient->state = tcpClient_kConnecting;
		createTimeEvent(tcpClient->eventLoop, tcpClient->retryDelayMs/1000.0, startConnectInLoop, tcpClient);
		tcpClient->retryDelayMs = tcpClient->retryDelayMs < kMaxRetryDelayMs ? tcpClient->retryDelayMs * 2 :  kMaxRetryDelayMs;
	}
}

void onClose(struct session* session, void* privateData) {
	myAssert(session != NULL);
	struct tcpClient* tcpClient = (struct tcpClient*)session->server;
	myAssert(tcpClient != NULL);
	onReConnect(tcpClient);
	tcpClient->session = NULL;
}

void onConnecting(struct aeEventLoop *eventLoop, int fd, void *clientData, int mask) {
	struct tcpClient* tcpClient = (struct tcpClient*)clientData;
	int error = 0;
	socklen_t len = sizeof(int);
	if ( (0 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len))) {
		if (0 == error){
			aeDeleteFileEvent(tcpClient->eventLoop->loop, fd, AE_WRITABLE|AE_READABLE);
			realConnect(tcpClient, fd);
		} else {
			aeDeleteFileEvent(tcpClient->eventLoop->loop, fd, AE_WRITABLE|AE_READABLE);
			onReConnect(tcpClient);
			close(fd);
		}
	} else {
		aeDeleteFileEvent(tcpClient->eventLoop->loop, fd, AE_WRITABLE|AE_READABLE);
		onReConnect(tcpClient);
		close(fd);
	}
}

void realConnect(struct tcpClient* tcpClient, fd_t fd) {
	myAssert(tcpClient != NULL);
	if (!tcpClient->session) {
		tcpClient->session = initSession();
		tcpClient->session->server = tcpClient;
	}
	tcpClient->session->mask = SESSION_READABLE;
	tcpClient->session->cfd = fd;
	tcpClient->session->callBackHandle->connectionCallBack = tcpClient->callBackHandler.connectionCallBack;
	tcpClient->session->callBackHandle->dataCallBack = tcpClient->callBackHandler.dataCallBack;
	tcpClient->session->callBackHandle->writeCompleteCallBack = tcpClient->callBackHandler.writeCompleteCallBack;
	tcpClient->session->callBackHandle->disConnectionCallBack = onClose;
	updateSessionEvent(tcpClient->eventLoop, tcpClient->session);
	tcpClient->state = tcpClient_kConnected;
	tcpClient->session->state = kConnected;
	if (tcpClient->callBackHandler.connectionCallBack) {
		tcpClient->callBackHandler.connectionCallBack(tcpClient->session, tcpClient->session->privateData);
	}
}

int32 startConnectInLoop(void* client) {
	struct tcpClient* tcpClient = (struct tcpClient*)client;
	myAssert(tcpClient != NULL);
	myAssert(tcpClient->eventLoop != NULL);

	if (tcpClient->eventLoop->state != EVENTLOOP_RUN) {
		mylog(LOG_INFO, "start connection in loop");
		return -1;
	}

	if (tcpClient->state != tcpClient_kConnecting) {
		return -1;
	}

	char err[256];
	int flag;
	fd_t fd = anetTcpConnectEx(err, tcpClient->serverAddr, tcpClient->serverPort, &flag);
	myAssert(fd != ANET_ERR);
    anetNonBlock(NULL,  fd);
    anetTcpNoDelay(NULL, fd);
    if (flag == 0) {
    	realConnect(tcpClient, fd);
    } else if (flag == 1) {
		aeCreateFileEvent(tcpClient->eventLoop->loop, fd, AE_READABLE | AE_WRITABLE, onConnecting, tcpClient);
    }

	return 0;
}

struct tcpClient* initTcpClient(struct eventLoop* loop, const char* serverAddr, uint32 serverPort) {
	struct tcpClient* client = (struct tcpClient*)zcalloc(sizeof(struct tcpClient));
	client->eventLoop = loop;
	client->serverAddr = zstrdup(serverAddr);
	client->serverPort = serverPort;
	client->state = tcpClient_kDisconnected;
	return client;
}

void destroyTcpClient(struct tcpClient* tcpClient) {
	myAssert(tcpClient != NULL);
	if (tcpClient->serverAddr) {
		zfree(tcpClient->serverAddr);
	}
	if (tcpClient->session) {
		destroySession(tcpClient->session);
	}
	zfree(tcpClient);
}

int32 startTcpClient(struct tcpClient* tcpClient) {
	myAssert(tcpClient != NULL);
	connectEvent(tcpClient->eventLoop, tcpClient);
}

void stopTcpClient(struct tcpClient* tcpClient) {
	myAssert(tcpClient != NULL);
	tcpClient->state = tcpClient_kDisconnected;
}

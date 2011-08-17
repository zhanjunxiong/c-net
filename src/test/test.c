/*
 * test.c
 *
 *  Created on: 2011-8-15
 *      Author: Administrator
 */
#include "../net/buffer.h"
#include "../net/callback.h"
#include "../net/err.h"
#include "../net/eventLoop.h"
#include "../net/tcpServer.h"
#include "../net/session.h"
#include "../net/mylog.h"

#include <signal.h>

void handleConnection(struct session* session, void* privateData) {
	mylog(LOG_INFO, "onConnection");
	char* test = "HELLO WORLD!!!";
	struct buffer* sendbuf = initBuffer(strlen(test));
	appendBuffer(sendbuf, test, strlen(test));
	writeSession(session, sendbuf);
	destroyBuffer(sendbuf);
}

RetHandle handleData(struct session* session, void* privateData) {
	mylog(LOG_INFO, "handleData!!!!");
	if (!session) {
		return CLOSE;
	}

	int readableBytes = getReadableBytesBuffer(session->readBuffer);
	struct buffer* sendbuf = initBuffer(readableBytes);
	appendBuffer(sendbuf, getReadIndexBuffer(session->readBuffer), readableBytes);
	mylog(LOG_INFO, "sendbuf, len: %d",  getReadableBytesBuffer(sendbuf));
	writeSession(session, sendbuf);
	moveReadIndexBuffer(session->readBuffer, readableBytes);
	destroyBuffer(sendbuf);
	return CONTINUE;
}

void handleDisConnection(struct session* session, void* privateData) {
	mylog(LOG_INFO, "onDisConnection");
}

struct eventLoop* baseLoop= NULL;
void onSigInt(int sig) {
	mylog(LOG_NOTICE, "onSigInt");
	if (baseLoop) {
		mylog(LOG_INFO, "=========");
		stopEventLoop(baseLoop);
	}
}
int main() {
	signal(SIGINT, onSigInt);
	baseLoop = initEventLoop(gettid());
	myAssert(baseLoop != NULL);
	int threadNum = 4;
	char* listenAddr = "127.0.0.1";
	int listenPort = 1208;
	struct tcpServer* server = initTcpServer(baseLoop, threadNum, listenAddr, listenPort);
	myAssert(server != NULL);
	server->callBack->connectionCallBack = handleConnection;
	server->callBack->dataCallBack = handleData;
	server->callBack->disConnectionCallBack = handleDisConnection;
	startTcpServer(server);
	startEventLoop(baseLoop);
	mylog(LOG_INFO, "START SERVER!!!");
	loopEventLoop(baseLoop);
	mylog(LOG_INFO, "STOP SERVER!!!");
	stopTcpServer(server);
	destroyTcpServer(server);
	destroyEventLoop(baseLoop);
	return 0;
}

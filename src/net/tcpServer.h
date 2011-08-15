/*
 * tcpServer.h
 *
 *  Created on: 2011-8-12
 *      Author:
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_

#ifdef __cpluscplus
extern "C" {
#endif

#include "dataType.h"

#include <pthread.h>

struct eventLoop;
struct eventLoopThreadPool;
struct fastArray;
struct ICallBack;

typedef struct tcpServer {
	struct eventLoop* baseLoop;
	struct eventLoopThreadPool* threadPool;
	fd_t listenfd;
	char* listenAddr;
	uint32 listenport;
	pthread_mutex_t sessionArrayLock;
	struct fastArray* sessionArray;
	struct ICallBack* callBack;
}tcpServer;

struct tcpServer* initTcpServer(struct eventLoop* baseLoop, uint32 threadNum, const char* listenAddr, uint32 listenPort);
void destroyTcpServer(struct tcpServer* tcpServer);

uint32 startTcpServer(struct tcpServer* server);
void stopTcpServer(struct tcpServer* server);

#ifdef __cpluscplus
}
#endif

#endif /* TCPSERVER_H_ */

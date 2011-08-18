/*
 * tcpClient.h
 *
 *  Created on: 2011-8-17
 *      Author:
 */

#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include "dataType.h"

struct eventLoop;
struct ICacllBack;
struct session;
enum tcpClientStates { tcpClient_kDisconnected, tcpClient_kConnecting, tcpClient_kConnected };

typedef struct tcpClient {
	struct eventLoop* eventLoop;
	char* serverAddr;
	uint32 serverPort;
	uint32 retryDelayMs;
	uint32 retry;
	ICallBack callBackHandler;
	struct session* session;
	enum tcpClientStates state;
} tcpClient;

struct tcpClient* initTcpClient(struct eventLoop* loop, const char* serverAddr, uint32 serverPort);
void destroyTcpClient(struct tcpClient* tcpClient);

int32 startTcpClient(struct tcpClient* tcpClient);
void stopTcpClient(struct tcpClient* tcpClient);

#endif /* TCPCLIENT_H_ */

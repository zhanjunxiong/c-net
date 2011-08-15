
#ifndef SESSION_H_
#define SESSION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "callback.h"
#include "dataType.h"
#include "fastArray.h"

#define IPADDR_LEN		128

#define INVALID_FD		-1

#define	NO_USE				0
#define 	IN_USE				1

#define SESSION_NONE	 		0
#define SESSION_READABLE 	1        //0001B
#define SESSION_WRITABLE 	2		 //0010B

struct arrayItem;
struct eventLoop;
struct buffer;

enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

typedef struct session {
	struct arrayItem arrayItem; //不能调整这个数据结构的位置

	fd_t cfd;
	uint32 mask;

	uint32 cport;
	uint8 cip[IPADDR_LEN];
	uint32 inUse;
	enum StateE	 state;

	struct buffer* readBuffer;
	struct buffer* sendBuffer;
	struct eventLoop* loop;
	struct ICallBack* callBackHandle;

    void* privateData;
    void* server;
} session;

struct session* initSession();
void destroySession(struct session* session);

int32 readSession(struct session* session);
int32 writeSession(struct session* session, struct buffer* sendBuf);

#ifdef __cplusplus
}
#endif

#endif /* SESSION_H_ */

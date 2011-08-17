
#ifndef CALLBACK_H_
#define CALLBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dataType.h"

typedef enum RetHandle {
	CONTINUE		=	0,
	SHUTDOWN	=	1,
	CLOSE				=	2
}RetHandle;

struct session;

typedef void onConnection(struct session* session, void* privateData);
typedef RetHandle onData(struct session* session, void* privateData);
typedef void onDisConnection(struct session* session, void* privateData);
typedef	void onWriteComplete(struct session* session);

typedef int32 onTimeCallBack(void* arg);

typedef struct ICallBack {
	onConnection* connectionCallBack;
	onData* dataCallBack;
	onDisConnection* disConnectionCallBack;
	onWriteComplete* writeCompleteCallBack;
} ICallBack;

#ifdef __cplusplus
}
#endif

#endif /* CALLBACK_H_ */


#include "buffer.h"
#include "err.h"
#include "fastArray.h"
#include "session.h"
#include "mylog.h"
#include "zmalloc.h"

#define READ_BUFFER_INITIAL									1024
#define SEND_BUFFER_INITIAL									1024

struct session* initSession() {
	struct session* session = (struct session* )zcalloc(sizeof(struct session));
	session->readBuffer = initBuffer(READ_BUFFER_INITIAL);
	session->cfd = INVALID_FD;
	session->inUse = NO_USE;
	session->mask = SESSION_READABLE; //默认可读？？
	session->arrayItem.arrayIndex = -1;
	session->state = kConnecting;
	session->callBackHandle = zcalloc(sizeof(struct ICallBack));
    return session;
}

void destroySession(struct session* session) {
	myAssert(session != NULL);
	session->state = kDisconnected;
	session->mask = SESSION_NONE;
	if (session->readBuffer) {
		destroyBuffer(session->readBuffer);
	}

	if (session->sendBuffer) {
		destroyBuffer(session->sendBuffer);
	}

	if (session->callBackHandle) {
		zfree(session->callBackHandle);
	}

	if (session->cfd) {
		close(session->cfd);
	}

	zfree(session);
}
//
int32 readSession(struct session* session) {
	myAssert(session != NULL);
	//if ( !(session->mask & SESSION_READABLE) ) {
	//	mylog(LOG_WARNING, "%s, %d, read session error, session can not read!!!\n", __FUNCTION__, __LINE__);
	//	return 0;
	//}
	int32 badRead = 1;
	int32 readLen = 0;
	while (1) {
		uint32 readBufferSize = getWriteableBytesBuffer(session->readBuffer);
		if (readBufferSize == 0) {
			expandBuffer(session->readBuffer);
			readBufferSize = getWriteableBytesBuffer(session->readBuffer);
		}
		int32 rlen = recv(session->cfd, getWriteIndexBuffer(session->readBuffer), readBufferSize, 0);
		if (rlen > 0) {
			moveWriteIndexBuffer(session->readBuffer, rlen);
			readLen += rlen;
		} else if (rlen == 0) {
			break;
		} else if (rlen == -1) {
			if (errno == EAGAIN) {
				break;
			} else {
				badRead = 0;
				break;
			}
		} else {
			badRead = 0;
			break;
		}
	}

	if (badRead == 0) {
		return 0;
	}
	return readLen;
}

int32 writeSession(struct session* session, struct buffer* sendBuf) {
	myAssert(session != NULL && sendBuf != NULL);
	if (session->state != kConnected) {
		mylog(LOG_WARNING, "%s, %d, read session error, session is not kconnected!!!\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (!(session->mask & SESSION_WRITABLE)) {
		while (1) {
			uint32 canReadSize = getReadableBytesBuffer(sendBuf);
			if (canReadSize <= 0) {
				break;
			}
			int32 writeLen = send(session->cfd, getReadIndexBuffer(sendBuf), canReadSize, 0);
			if (writeLen > 0) {
				moveReadIndexBuffer(sendBuf, writeLen);
			} else if (writeLen == -1) {
				if (errno == EAGAIN) {
					break;
				} else {
					return -3;
				}
			} else {
				return -4;
			}
		}
	}

	uint32 canReadSize = getReadableBytesBuffer(sendBuf);
	if (canReadSize > 0) {
		if (!session->sendBuffer) {
			session->sendBuffer = initBuffer(SEND_BUFFER_INITIAL);
		}
		appendBuffer(session->sendBuffer, getReadIndexBuffer(sendBuf), canReadSize);
		session->mask = session->mask & SESSION_WRITABLE;
		updateSessionEvent(session->loop, session);
	} else {
		if (session->callBackHandle) {
			if (session->callBackHandle->writeCompleteCallBack) {
				session->callBackHandle->writeCompleteCallBack(session);
			}
		}
	}
	return 0;
}

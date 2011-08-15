
#include "err.h"
#include "buffer.h"
#include "mylog.h"
#include "zmalloc.h"

static const uint32 kCheapPrepend = 8;

struct buffer* initBuffer(uint32 initSize) {
	struct buffer* buf = NULL;
	buf = (struct buffer*)zcalloc(sizeof(struct buffer));
	buf->data = NULL;
	buf->readIndex = kCheapPrepend;
	buf->writeIndex = kCheapPrepend;
	buf->size = initSize;
	if (initSize == 0) {
		return buf;
	}

	buf->data = (char* )zmalloc(kCheapPrepend + initSize);
	return buf;
}

void destroyBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	if (buf->data) {
		zfree(buf->data);
	}
	zfree(buf);
}

char* getWriteIndexBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return beginBuffer(buf) + buf->writeIndex;
}

char* getReadIndexBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return beginBuffer(buf) + buf->readIndex;
}

void moveWriteIndexBuffer(struct buffer* buf, uint32 moveLen) {
	myAssert(buf != NULL);
	buf->writeIndex += moveLen;
}

void moveReadIndexBuffer(struct buffer* buf, uint32 moveLen) {
	myAssert(buf != NULL);
	buf->readIndex += moveLen;
}

char* beginBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return (char*)buf->data;
}
//
void makeSpaceBuffer(struct buffer* buf, uint32 more) {
	myAssert(buf != NULL);
	if (!buf->data) {
		buf->data = (char*)zmalloc(more + kCheapPrepend);
		buf->writeIndex = kCheapPrepend;
		buf->readIndex = kCheapPrepend;
	} else if (getWriteableBytesBuffer(buf) + prependableBytesBuffer(buf)  < more + kCheapPrepend) {
		expandBuffer(buf);
	} else {
		uint32 used = getReadableBytesBuffer(buf);
		memmove(beginBuffer(buf) + kCheapPrepend, getReadIndexBuffer(buf), used);
		buf->readIndex = kCheapPrepend;
		buf->writeIndex = buf->readIndex + used;
	}
}

void appendBuffer(struct buffer* buf, const char* data, uint32 dataLen) {
	myAssert(buf != NULL);
	if (dataLen > getWriteableBytesBuffer(buf)) {
		makeSpaceBuffer(buf, dataLen);
	}

	memcpy(beginBuffer(buf) + buf->writeIndex, data, dataLen);
	buf->writeIndex += dataLen;
}

void prependBuffer(struct buffer* buf, const char* data, uint32 dataLen) {
	myAssert(buf != NULL);
	if (dataLen <= prependableBytesBuffer(buf)) {
		mylog(LOG_WARNING, "buf prependableBytesBuffer error, prependableBytes[%d], dataLen[%d]!!!", prependableBytesBuffer(buf), dataLen);
		return;
	}

	uint32 readIndex = buf->readIndex;
	readIndex -= dataLen;
	memcpy(beginBuffer(buf) + readIndex, data, dataLen);
	buf->readIndex = readIndex;
}

uint32 expandBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	int reallocSize = buf->size << 1;
	char* tmp = (char*)zrealloc(buf->data, reallocSize * sizeof(char));
	buf->size = reallocSize;
	buf->data = tmp;
	return 0;
}

uint32 getWriteableBytesBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return buf->size - buf->writeIndex;
}

uint32 getReadableBytesBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return buf->writeIndex - buf->readIndex;
}

void retrieveAllBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	buf->writeIndex = kCheapPrepend;
	buf->readIndex = kCheapPrepend;
}

uint32 prependableBytesBuffer(struct buffer* buf) {
	myAssert(buf != NULL);
	return buf->readIndex;
}


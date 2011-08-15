
#ifndef BUFFER_H_
#define BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dataType.h"

typedef struct buffer {
	char* data;
	uint32	size;
	uint32	writeIndex;
	uint32	readIndex;
} buffer;

//构造和析构buffer
struct buffer* initBuffer(uint32 initSize);
void destroyBuffer(struct buffer* buf);
//获得buffer的写下标和读下标
char* getWriteIndexBuffer(struct buffer* buf);
char* getReadIndexBuffer(struct buffer* buf);
//移动buffer的写下标和读下标
void moveWriteIndexBuffer(struct buffer* buf, uint32 moveLen);
void moveReadIndexBuffer(struct buffer* buf, uint32 moveLen);
//往后或往前追加数据
void appendBuffer(struct buffer* buf, const char* data, uint32 dataLen);
void prependBuffer(struct buffer* buf, const char* data, uint32 dataLen);
// 扩展buffer的容量
uint32 expandBuffer(struct buffer* buf);
//获得buffer可读和可写的字节数
uint32 getReadableBytesBuffer(struct buffer* buf);
uint32 getWriteableBytesBuffer(struct buffer* buf);
//
void retrieveAllBuffer(struct buffer* buf);
uint32 prependableBytesBuffer(struct buffer* buf);
//
char* beginBuffer(struct buffer* buf);

#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H_ */

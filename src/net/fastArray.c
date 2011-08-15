
#include "err.h"
#include "fastArray.h"
#include "zmalloc.h"

struct fastArray* initFastArray(uint32 capacity) {
	struct fastArray* array = (struct fastArray*)zcalloc(sizeof(struct fastArray));
	array->arrayItem = (void**)zcalloc(sizeof(void *) * capacity);
	array->capacity = capacity > 1 ? capacity : 2;
	array->tail = 0;
	return array;
}

void destroyFastArray(struct fastArray* fastArray) {
	myAssert(fastArray != NULL);
	if (fastArray->arrayItem) {
		zfree(fastArray->arrayItem);
	}
	zfree(fastArray);
}

static int32 pushBack(struct fastArray* fastArray, void* item) {
	myAssert(fastArray != NULL && item != NULL);
	if (fastArray->tail >= fastArray->capacity) {
		myAssert(fastArray->tail > 0);
		uint32 capacity = fastArray->tail << 1;
		fastArray->arrayItem = (void**) zrealloc(fastArray->arrayItem, capacity * sizeof(void*));
		fastArray->capacity = capacity;
	}

	fastArray->arrayItem[fastArray->tail] = item;
	fastArray->tail++;
	return 0;
}

static int32 erase(struct fastArray* fastArray, int arrayIndex) {
	myAssert(fastArray != NULL && (arrayIndex >= 0 && arrayIndex < fastArray->tail) );
	uint32 tail = fastArray->tail;
	if (arrayIndex == tail - 1) {
		fastArray->tail--;
		return 0;
	}

	struct arrayItem* tailItem = (struct arrayItem* )fastArray->arrayItem[tail - 1];
	fastArray->arrayItem[arrayIndex] = tailItem;
	fastArray->tail--;
	return 0;
}

void pushBackFastArray(struct fastArray* fastArray, struct arrayItem* item) {
	myAssert(fastArray != NULL && item != NULL);
	item->arrayIndex = fastArray->tail;
	pushBack(fastArray, item);
}

void eraseFastArray(struct fastArray* fastArray, struct arrayItem* item) {
	myAssert(fastArray != NULL && item != NULL);
	int32 arrayIndex = item->arrayIndex;
	myAssert(arrayIndex >= 0 && arrayIndex < fastArray->tail);
	erase(fastArray, arrayIndex);
	item->arrayIndex = -1;
	return;
}

uint32 getTailFastArray(struct fastArray* fastArray) {
	myAssert(fastArray != NULL);
	return fastArray->tail;
}

void* getArrayItemFastArray(struct fastArray* fastArray, int32 arrayIndex) {
	myAssert(fastArray != NULL);
	if ( arrayIndex < 0 || arrayIndex > fastArray->tail) {
		return NULL;
	}

	return fastArray->arrayItem[arrayIndex];
}

int32 getArrayIndexFastArray(struct arrayItem* item) {
	myAssert(item != NULL);
	return item->arrayIndex;
}

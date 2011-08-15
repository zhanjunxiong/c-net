
#ifndef FASTARRAY_H_
#define FASTARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "dataType.h"

typedef struct arrayItem {
	int32 arrayIndex;
} arrayItem;

typedef struct fastArray {
	uint32 capacity;
	uint32 tail;
	void** arrayItem;
} fastArray;

struct fastArray* initFastArray(uint32 capacity);
void destroyFastArray(struct fastArray* fastArray);

void pushBackFastArray(struct fastArray* fastArray, struct arrayItem* item);
void eraseFastArray(struct fastArray* fastArray, struct arrayItem* item);
uint32 getTailFastArray(struct fastArray* fastArray);
void* getArrayItemFastArray(struct fastArray* fastArray, int32 arrayIndex);

int32 getArrayIndexFastArray(struct arrayItem* item);

#ifdef __cplusplus
}
#endif

#endif /* FASTARRAY_H_ */

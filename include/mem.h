#ifndef MEM_H_
#define MEM_H_

#include "types.h"

void *Alloc(usize size);
void *Realloc(void *ptr, usize size);
void Free(void *ptr);

#endif /* MEM_H_ */
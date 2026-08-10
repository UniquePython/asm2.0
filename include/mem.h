#ifndef MEM_H_
#define MEM_H_

#include "types.h"

void *alloc(usize size);
void *realloc_(void *ptr, usize size);

#endif /* MEM_H_ */
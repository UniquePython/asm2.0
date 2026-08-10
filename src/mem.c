#include "mem.h"
#include "diagnostic.h"

#include <stdlib.h>

void *alloc(usize size)
{
    void *ptr = malloc(size);
    if (!ptr)
        Error("out of memory");
    return ptr;
}

void *realloc_(void *ptr, usize size)
{
    void *newPtr = realloc(ptr, size);
    if (!newPtr)
        Error("out of memory");
    return newPtr;
}

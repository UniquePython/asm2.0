#include "mem.h"
#include "diagnostic.h"

#include <stdlib.h>

void *Alloc(usize size)
{
    void *ptr = malloc(size);
    if (!ptr)
        Error("out of memory");
    return ptr;
}

void *Realloc(void *ptr, usize size)
{
    void *newPtr = realloc(ptr, size);
    if (!newPtr)
        Error("out of memory");
    return newPtr;
}

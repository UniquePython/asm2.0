#ifndef SOURCE_H_
#define SOURCE_H_

#include "types.h"

typedef struct source_t
{
    char *filepath;
    char *data;
    u32 len;
} Source;

bool SourceLoad(const char *filepath, Source *out);
void SourceFree(Source *source);

#endif
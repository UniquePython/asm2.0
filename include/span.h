#ifndef SPAN_H_
#define SPAN_H_

#include "types.h"

typedef struct span_t
{
    u32 start;
    u32 end;
} Span;

static inline u32 SpanLength(Span span)
{
    return span.end - span.start;
}

#endif /* SPAN_H_ */

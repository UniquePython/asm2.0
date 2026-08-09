#ifndef SPAN_H_
#define SPAN_H_

#include "types.h"

#define SPAN_BITS 16

#if SPAN_BITS == 16
typedef u16 SpanOffset;
#elif SPAN_BITS == 32
typedef u32 SpanOffset;
#elif SPAN_BITS == 64
typedef u64 SpanOffset;
#else
#error "SPAN_BITS must be 16, 32, or 64"
#endif

typedef struct span_t
{
    SpanOffset start;
    SpanOffset end;
} Span;

static inline SpanOffset SpanLength(Span span)
{
    return span.end - span.start;
}

#endif /* SPAN_H_ */

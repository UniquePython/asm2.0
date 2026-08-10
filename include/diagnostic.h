#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "types.h"
#include "span.h"
#include "source.h"

void Error(const char *fmt, ...);
void InternalError(const char *fmt, ...);
void LexError(const Source *source, Span span, const char *fmt, ...);
void ParseError(const Source *source, Span span, const char *fmt, ...);

#endif /* DIAGNOSTIC_H_ */

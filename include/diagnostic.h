#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "types.h"
#include "span.h"
#include "source.h"

void Error(const char *fmt, ...);
void InternalError(const char *fmt, ...);
void ParseError(const Source *source, Span span, const char *fmt, ...);
void CodegenError(const Source *source, Span span, const char *fmt, ...);

typedef struct diag_entry_t
{
    Span span;
    char *message; /* owned, heap-allocated, already formatted */
} DiagEntry;

typedef struct diags_t
{
    DiagEntry *entries;
    usize len;
    usize cap;
} Diags;

void DiagsInit(Diags *diags);
void DiagsFree(Diags *diags);
void DiagsPush(Diags *diags, Span span, const char *fmt, ...);
void DiagsReportAll(const Diags *diags, const Source *source, const char *prefix);

#endif /* DIAGNOSTIC_H_ */

#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "types.h"
#include "span.h"
#include "source.h"

void Error(const char *fmt, ...);
void InternalError(const char *fmt, ...);
void CodegenError(const Source *source, Span span, const char *fmt, ...);

/*
 * Same as CodegenError, but also accepts an optional help message and
 * an optional syntax reminder, printed as "help:" (green) and
 * "syntax:" (yellow) lines after the source snippet.
 *
 * `help`, if non-NULL, is expected to be a heap-allocated string (e.g.
 * from HelpMessage) -- ownership transfers in and it is freed before
 * this function exits (it's fatal, so nothing else can free it later).
 * Pass NULL for no help line.
 *
 * `syntax`, if non-NULL, is expected to be a borrowed static string
 * literal (e.g. one of the *_SYNTAX macros in syntax.h) -- it is never
 * freed. Pass NULL for no syntax line.
 */
void CodegenErrorFull(const Source *source, Span span, char *help, const char *syntax, const char *fmt, ...);

/*
 * Formats a help message, e.g. HelpMessage("did you mean '%s'?", sugg).
 * Returned string is heap-allocated and its ownership transfers to
 * whichever diagnostic call it is passed into (DiagsPushFull or
 * CodegenErrorFull) -- the caller must not free it themselves.
 */
char *HelpMessage(const char *fmt, ...);

typedef struct diag_entry_t
{
    Span span;
    char *message;      /* owned, heap-allocated, already formatted */
    char *help;         /* owned */
    const char *syntax; /* borrowed literal from syntax.h */
} DiagEntry;

typedef struct diags_t
{
    DiagEntry *entries;
    usize len;   /* number of diagnostics actually stored (<= max) */
    usize cap;   /* allocated capacity of entries[] (<= max) */
    usize max;   /* storage cap; 0 means unlimited */
    usize nerrs; /* total DiagsPush calls, including suppressed ones */
} Diags;

void DiagsInit(Diags *diags, usize max);
void DiagsFree(Diags *diags);
void DiagsPush(Diags *diags, Span span, const char *fmt, ...);
void DiagsPushFull(Diags *diags, Span span, char *help, const char *syntax, const char *fmt, ...);

void DiagsReportAll(const Diags *diags, const Source *source, const char *prefix);

#endif /* DIAGNOSTIC_H_ */

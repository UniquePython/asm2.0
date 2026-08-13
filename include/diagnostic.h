#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "types.h"
#include "span.h"
#include "source.h"

typedef enum severity_t
{
    SEVERITY_WARNING,
    SEVERITY_ERROR,
} Severity;

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
    Severity severity;  /* SEVERITY_ERROR or SEVERITY_WARNING -- controls
                         * both the printed prefix color and whether this
                         * entry counts toward Diags::nerrs. */
    char *message;      /* owned, heap-allocated, already formatted */
    char *help;         /* owned */
    const char *syntax; /* borrowed literal from syntax.h */
} DiagEntry;

typedef struct diags_t
{
    DiagEntry *entries;
    usize len;       /* number of diagnostics actually stored (<= max) */
    usize cap;       /* allocated capacity of entries[] (<= max) */
    usize max;       /* storage cap; 0 means unlimited */
    usize nerrs;     /* total error-severity DiagsPush calls, including
                      * suppressed ones -- callers should key pipeline
                      * abort decisions off this, NOT off len, since len
                      * also counts warnings and warnings must not halt
                      * the pipeline. */
    usize nwarnings; /* total warning-severity DiagsPush calls, including
                      * suppressed ones. Purely informational -- nothing
                      * in this module treats it as fatal. */
} Diags;

void DiagsInit(Diags *diags, usize max);
void DiagsFree(Diags *diags);

/*
 * DiagsPush / DiagsPushFull push an ERROR-severity diagnostic.
 * DiagsPushWarning / DiagsPushWarningFull push a WARNING-severity one.
 * All four share the same storage, capacity, and reporting path --
 * only the severity (and therefore which counter it bumps, and which
 * color it prints in) differs. See DiagsPushFull's existing doc
 * comment for the `help` / `syntax` ownership rules; they apply
 * identically to the warning variants.
 */
void DiagsPush(Diags *diags, Span span, const char *fmt, ...);
void DiagsPushFull(Diags *diags, Span span, char *help, const char *syntax, const char *fmt, ...);
void DiagsPushWarning(Diags *diags, Span span, const char *fmt, ...);
void DiagsPushWarningFull(Diags *diags, Span span, char *help, const char *syntax, const char *fmt, ...);

void DiagsReportAll(const Diags *diags, const Source *source, const char *prefix);

#endif /* DIAGNOSTIC_H_ */

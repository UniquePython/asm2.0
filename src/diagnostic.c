#include "diagnostic.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#define ANSI_RESET "\x1b[0m"
#define ANSI_RED "\x1b[31m"
#define ANSI_BOLD "\x1b[1m"
#define ANSI_CYAN "\x1b[36m"

static bool DiagnosticUseColor(void)
{
    static bool initialized = false;
    static bool enabled = false;

    if (!initialized)
    {
        enabled = isatty(STDERR_FILENO);
        initialized = true;
    }

    return enabled;
}

static const char *Color(const char *color)
{
    return DiagnosticUseColor() ? color : "";
}

static const char *ResetColor(void)
{
    return DiagnosticUseColor() ? ANSI_RESET : "";
}

static void ReportSimple(const char *prefix, int exitCode, const char *fmt, va_list args)
{
    fprintf(stderr, "%s%s:%s ", Color(ANSI_RED), prefix, ResetColor());
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    exit(exitCode);
}

static void PositionFromOffset(const Source *source, u32 offset, u32 *line, u32 *col)
{
    if (offset > source->len)
        InternalError("offset greater than length of source code");

    u32 currLine = 1;
    u32 currCol = 1;

    for (u32 idx = 0; idx < offset; idx++)
    {
        if (source->data[idx] == '\n')
        {
            currLine++;
            currCol = 1;
        }
        else
            currCol++;
    }

    *line = currLine;
    *col = currCol;
}

static void FindSourceLine(const Source *source, u32 offset, u32 *lineStart, u32 *lineLen)
{
    if (offset > source->len)
        InternalError("offset greater than length of source code");

    u32 start = offset;
    while (start > 0 && source->data[start - 1] != '\n')
        start--;

    u32 end = offset;
    while (end < source->len && source->data[end] != '\n')
        end++;

    *lineStart = start;
    *lineLen = end - start;
}

static void PrintGutter(FILE *out, u32 line, u32 width, bool with_number)
{
    if (with_number)
        fprintf(out, "%*u | ", (int)width, line);
    else
        fprintf(out, "%*s | ", (int)width, "");
}

/*
 * Prints one positioned diagnostic (message, "--> file:line:col", source
 * line, caret underline) to stderr. Does NOT exit -- shared by the fatal
 * single-error path (ReportPositioned) and the Diags batch-report path.
 */
static void PrintPositioned(const char *prefix, const Source *source, Span span, const char *message)
{
    u32 line;
    u32 col;

    PositionFromOffset(source, span.start, &line, &col);

    /* <prefix>: <message> */
    fprintf(stderr, "%s%s:%s %s\n", Color(ANSI_RED), prefix, ResetColor(), message);

    /* --> filepath:line:col */
    fprintf(
        stderr,
        "  %s-->%s %s%s%s:" U32_FMT ":" U32_FMT "\n",
        Color(ANSI_BOLD),
        ResetColor(),
        Color(ANSI_CYAN),
        source->filepath,
        ResetColor(),
        line,
        col);

    u32 lineStart, lineLen;
    FindSourceLine(source, span.start, &lineStart, &lineLen);

    /* Calculate gutter width. */
    u32 gutterWidth = 1;
    for (u32 n = line; n >= 10; n /= 10)
        gutterWidth++;

    /* Source line. */
    PrintGutter(stderr, line, gutterWidth, true);
    fprintf(stderr, "%.*s\n", (int)lineLen, source->data + lineStart);

    /* Underline. */
    PrintGutter(stderr, line, gutterWidth, false);

    /*
     * col is 1-based, so we need col - 1 spaces before
     * the first '^'.
     */
    for (u32 i = 1; i < col; i++)
        fputc(' ', stderr);

    u32 spanLen = SpanLength(span);

    /*
     * Make sure EOF / zero-length spans still produce something
     * visible.
     */
    if (spanLen == 0)
        spanLen = 1;

    fprintf(stderr, "%s", Color(ANSI_RED));
    for (u32 i = 0; i < spanLen; i++)
        fputc('^', stderr);
    fprintf(stderr, "%s\n", ResetColor());
}

static void ReportPositioned(const char *prefix, int exitCode, const Source *source, Span span, const char *fmt, va_list args)
{
    va_list argsCopy;
    va_copy(argsCopy, args);
    int n = vsnprintf(NULL, 0, fmt, argsCopy);
    va_end(argsCopy);

    if (n < 0)
        InternalError("failed to format diagnostic message");

    usize len = (usize)n + 1;
    char *message = malloc(len);
    if (!message)
        Error("out of memory");

    vsnprintf(message, len, fmt, args);

    PrintPositioned(prefix, source, span, message);

    exit(exitCode);
}

void Error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ReportSimple("error", 2, fmt, args);
    va_end(args);
}

void InternalError(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ReportSimple("internal error", 1, fmt, args);
    va_end(args);
}

void CodegenError(const Source *source, Span span, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    ReportPositioned("codegen error", 5, source, span, fmt, args);
    va_end(args);
}

#define DIAGS_INITIAL_CAP 8

void DiagsInit(Diags *diags, usize max)
{
    diags->entries = NULL;
    diags->len = 0;
    diags->cap = 0;
    diags->max = max;
    diags->nerrs = 0;
}

void DiagsFree(Diags *diags)
{
    for (usize i = 0; i < diags->len; i++)
        free(diags->entries[i].message);

    free(diags->entries);

    diags->entries = NULL;
    diags->len = 0;
    diags->cap = 0;
}

void DiagsPush(Diags *diags, Span span, const char *fmt, ...)
{
    diags->nerrs++;

    if (diags->max > 0 && diags->len >= diags->max)
        return; /* at capacity -- don't format or store, just count */

    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (n < 0)
        InternalError("failed to format diagnostic message");

    usize len = (usize)n + 1;
    char *message = malloc(len);
    if (!message)
        Error("out of memory");

    va_start(args, fmt);
    vsnprintf(message, len, fmt, args);
    va_end(args);

    if (diags->len == diags->cap)
    {
        usize newCap = diags->cap == 0 ? DIAGS_INITIAL_CAP : diags->cap * 2;

        if (diags->max > 0 && newCap > diags->max)
            newCap = diags->max;

        DiagEntry *newEntries = realloc(diags->entries, newCap * sizeof(*diags->entries));
        if (!newEntries)
            Error("out of memory");

        diags->entries = newEntries;
        diags->cap = newCap;
    }

    diags->entries[diags->len] = (DiagEntry){
        .span = span,
        .message = message,
    };
    diags->len++;
}

void DiagsReportAll(const Diags *diags, const Source *source, const char *prefix)
{
    for (usize i = 0; i < diags->len; i++)
        PrintPositioned(prefix, source, diags->entries[i].span, diags->entries[i].message);

    if (diags->nerrs > diags->len)
    {
        usize suppressed = diags->nerrs - diags->len;
        fprintf(stderr, "%s%s:%s " U64_FMT " further error%s suppressed\n",
                Color(ANSI_RED), prefix, ResetColor(),
                (u64)suppressed, suppressed == 1 ? "" : "s");
    }
}

#include "stmt.h"

#include <stdio.h>

#include "mem.h"

Stmt NewLabelStmt(const char *name, Span nameSpan, Span span)
{
    return (Stmt){
        .kind = STMT_LABEL,
        .span = span,
        .as.label = {
            .name = name,
            .nameSpan = nameSpan,
        },
    };
}

Stmt NewMoveStmt(Operand src, Operand dest, Span span)
{
    return (Stmt){
        .kind = STMT_MOVE,
        .span = span,
        .as.move = {
            .src = src,
            .dest = dest,
        },
    };
}

Stmt NewSyscallStmt(Span span)
{
    return (Stmt){
        .kind = STMT_SYSCALL,
        .span = span,
    };
}

Stmt NewEntryStmt(const char *label, Span labelSpan, Span span)
{
    return (Stmt){
        .kind = STMT_ENTRY,
        .span = span,
        .as.entry = {
            .label = label,
            .labelSpan = labelSpan,
        },
    };
}

#define SPAN_FMT " @" U32_FMT ":" U32_FMT
#define SPAN_ARG(span) (span).start, (span).end
static int StmtFormat(const Stmt *stmt, char *buffer, usize len)
{
    switch (stmt->kind)
    {
    case STMT_LABEL:
        return snprintf(buffer, len, "Stmt label(\"%s\")" SPAN_FMT,
                        stmt->as.label.name, SPAN_ARG(stmt->span));

    case STMT_MOVE:
    {
        usize srcLen = OperandAsStrLen(&stmt->as.move.src);
        usize destLen = OperandAsStrLen(&stmt->as.move.dest);

        char *srcStr = Alloc(srcLen);
        char *destStr = Alloc(destLen);

        OperandAsStr(&stmt->as.move.src, srcStr);
        OperandAsStr(&stmt->as.move.dest, destStr);

        int n = snprintf(buffer, len, "Stmt move(%s, %s)" SPAN_FMT,
                         srcStr, destStr, SPAN_ARG(stmt->span));

        Free(srcStr);
        Free(destStr);

        return n;
    }

    case STMT_SYSCALL:
        return snprintf(buffer, len, "Stmt syscall" SPAN_FMT, SPAN_ARG(stmt->span));

    case STMT_ENTRY:
        return snprintf(buffer, len, "Stmt entry(\"%s\")" SPAN_FMT,
                        stmt->as.entry.label, SPAN_ARG(stmt->span));

    default:
        return snprintf(buffer, len, "Stmt %s" SPAN_FMT, "unknown", SPAN_ARG(stmt->span));
    }
}
#undef SPAN_ARG
#undef SPAN_FMT

usize StmtAsStrLen(const Stmt *stmt)
{
    int n = StmtFormat(stmt, NULL, 0);
    if (n < 0)
        return 0;
    return (usize)n + 1;
}

bool StmtAsStr(const Stmt *stmt, char *buffer)
{
    usize len = StmtAsStrLen(stmt);
    if (len == 0)
        return false;
    return StmtFormat(stmt, buffer, len) > 0 ? true : false;
}

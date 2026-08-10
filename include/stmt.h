#ifndef STMT_H_
#define STMT_H_

#include "types.h"
#include "span.h"
#include "operand.h"
#include "stmtkind.h"

typedef struct stmt_t
{
    StmtKind kind;
    Span span;

    union
    {
        struct
        {
            const char *name;
            Span nameSpan;
        } label;

        struct
        {
            Operand src;
            Operand dest;
        } move;

        struct
        {
            const char *label;
            Span labelSpan;
        } entry;
    } as;
} Stmt;

Stmt NewLabelStmt(const char *name, Span nameSpan, Span span);
Stmt NewMoveStmt(Operand src, Operand dest, Span span);
Stmt NewSyscallStmt(Span span);
Stmt NewEntryStmt(const char *label, Span labelSpan, Span span);

usize StmtAsStrLen(const Stmt *stmt);
bool StmtAsStr(const Stmt *stmt, char *buffer);

#endif /* STMT_H_ */

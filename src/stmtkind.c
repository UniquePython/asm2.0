#include "stmtkind.h"

const char *StmtKindAsStr(StmtKind kind)
{
    switch (kind)
    {
    case STMT_LABEL:
        return "label";

    case STMT_MOVE:
        return "move";

    case STMT_SYSCALL:
        return "syscall";

    default:
        return "unknown";
    }
}

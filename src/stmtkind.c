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

    case STMT_ENTRY:
        return "entry";

    default:
        return "unknown";
    }
}

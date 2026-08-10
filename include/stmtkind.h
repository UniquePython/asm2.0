#ifndef STMTKIND_H_
#define STMTKIND_H_

typedef enum stmt_kind_t
{
    STMT_LABEL,
    STMT_MOVE,
    STMT_SYSCALL,
} StmtKind;

const char *StmtKindAsStr(StmtKind kind);

#endif /* STMTKIND_H_ */

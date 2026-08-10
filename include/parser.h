#ifndef PARSER_H_
#define PARSER_H_

#include "stmt.h"

typedef struct stmt_node_t
{
    Stmt stmt;
    struct stmt_node_t *next;
} StmtNode;

#endif /* PARSER_H_ */

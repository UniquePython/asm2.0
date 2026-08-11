#ifndef PARSER_H_
#define PARSER_H_

#include "types.h"
#include "source.h"
#include "diagnostic.h"
#include "lexer.h"
#include "keyword.h"
#include "tokenkind.h"
#include "token.h"
#include "stmt.h"

typedef struct stmt_node_t
{
    Stmt stmt;
    struct stmt_node_t *next;
} StmtNode;

typedef struct parser_t
{
    const TokenNode *cursor;
    const Source *source;
    Diags *diags;
} Parser;

StmtNode *Parse(const Source *source, TokenNode *tokens, Diags *diags);
void ParseFree(StmtNode **head);

#endif /* PARSER_H_ */

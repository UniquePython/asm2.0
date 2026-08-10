#ifndef LEXER_H_
#define LEXER_H_

#include "types.h"
#include "source.h"
#include "token.h"

typedef struct token_node_t
{
    Token token;
    struct token_node_t *next;
} TokenNode;

TokenNode *Lex(const Source *source);
void LexFree(TokenNode **head);

#endif /* LEXER_H_ */

#ifndef TOKEN_H_
#define TOKEN_H_

#include "span.h"
#include "tokenkind.h"

typedef struct token_t
{
    TokenKind tk;
    Span span;
} Token;

#endif /* TOKEN_H_ */

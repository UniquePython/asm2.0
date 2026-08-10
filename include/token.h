#ifndef TOKEN_H_
#define TOKEN_H_

#include "types.h"
#include "span.h"
#include "keyword.h"
#include "tokenkind.h"

typedef struct token_t
{
    TokenKind tk;
    Span span;

    union token_as_t
    {
        Keyword keyword;
        u64 number;
        const char *identifier;
    } as;
} Token;

usize TokenAsStrLen(const Token *token);
bool TokenAsStr(const Token *token, char *buffer);

#endif /* TOKEN_H_ */

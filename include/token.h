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
        char *identifier;
        Keyword keyword;
        u64 number;
    } as;
} Token;

Token NewIdentifierToken(const char *identifier, u32 start, u32 end);
Token NewKeywordToken(Keyword keyword, u32 start, u32 end);
Token NewNumberToken(u64 number, u32 start, u32 end);
Token NewSimpleToken(TokenKind tk, u32 start, u32 end);
Token NewEOFToken(u32 start);

usize TokenAsStrLen(const Token *token);
bool TokenAsStr(const Token *token, char *buffer);

#endif /* TOKEN_H_ */

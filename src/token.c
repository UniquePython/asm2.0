#include "token.h"

#include <stdio.h>

#define SPAN_FMT " @" U32_FMT ":" U32_FMT
#define SPAN_ARG(span) (span).start, (span).end
static int TokenFormat(const Token *token, char *buffer, usize len)
{
    switch (token->tk)
    {
    case TK_IDENTIFIER:
        return snprintf(buffer, len, "Token identifier(\"%s\")" SPAN_FMT, token->as.identifier, SPAN_ARG(token->span));

    case TK_KEYWORD:
        return snprintf(buffer, len, "Token keyword(%s)" SPAN_FMT, KeywordAsStr(token->as.keyword), SPAN_ARG(token->span));

    case TK_NUMBER:
        return snprintf(buffer, len, "Token number(" U64_FMT ")" SPAN_FMT, token->as.number, SPAN_ARG(token->span));

    case TK_COLON:
    case TK_SEMICOLON:
    case TK_EOF:
        return snprintf(buffer, len, "Token %s" SPAN_FMT, TokenKindAsStr(token->tk), SPAN_ARG(token->span));

    default:
        return snprintf(buffer, len, "Token %s" SPAN_FMT, "unknown", SPAN_ARG(token->span));
    }
}
#undef SPAN_ARG
#undef SPAN_FMT

usize TokenAsStrLen(const Token *token)
{
    int n = TokenFormat(token, NULL, 0);
    if (n < 0)
        return 0;
    return (usize)n + 1;
}

bool TokenAsStr(const Token *token, char *buffer)
{
    usize len = TokenAsStrLen(token);
    if (len == 0)
        return false;
    return TokenFormat(token, buffer, len) > 0 ? true : false;
}

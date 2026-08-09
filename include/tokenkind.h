#ifndef TOKENKIND_H_
#define TOKENKIND_H_

typedef enum tokenkind_t
{
    TK_IDENTIFIER,
    TK_KEYWORD,
    TK_NUMBER,
    TK_COLON,
    TK_SEMICOLON,
    TK_EOF,
} TokenKind;

const char *TokenKindAsStr(TokenKind tk);

#endif /* TOKENKIND_H_ */

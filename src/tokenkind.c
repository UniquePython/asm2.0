#include "tokenkind.h"

const char *TokenKindAsStr(TokenKind tk)
{
    switch (tk)
    {
    case TK_IDENTIFIER:
        return "identifier";

    case TK_KEYWORD:
        return "keyword";

    case TK_NUMBER:
        return "number";

    case TK_COLON:
        return "colon";

    case TK_SEMICOLON:
        return "semicolon";

    case TK_EOF:
        return "end of file";

    default:
        return "unknown";
    }
}

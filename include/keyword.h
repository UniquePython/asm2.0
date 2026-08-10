#ifndef KEYWORD_H_
#define KEYWORD_H_

#include "types.h"

typedef enum keyword_t
{
    KW_LABEL,
    KW_MOVE,
    KW_TO,
    KW_SYSCALL,
    NKWS,
} Keyword;

bool StrToKeyword(const char *str, usize len, Keyword *out);

const char *KeywordAsStr(Keyword kw);

#endif /* KEYWORD_H_ */
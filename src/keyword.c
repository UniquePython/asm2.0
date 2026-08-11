#include "keyword.h"

#include <string.h>
#include <assert.h>

static const char *keywords[] = {
    "label",
    "move",
    "to",
    "syscall",
    "entry",
};

static_assert(sizeof(keywords) / sizeof(keywords[0]) == NKWS, "keywords[] must have exactly NKWS entries, in enum order");

bool StrToKeyword(const char *str, usize len, Keyword *out)
{
    for (usize idx = 0; idx < NKWS; idx++)
    {
        const char *keyword = keywords[idx];
        if (strlen(keyword) == len && strncmp(keyword, str, len) == 0)
        {
            *out = (Keyword)idx;
            return true;
        }
    }

    return false;
}

const char *KeywordAsStr(Keyword kw)
{
    switch (kw)
    {
    case KW_LABEL:
        return "label";

    case KW_MOVE:
        return "move";

    case KW_TO:
        return "to";

    case KW_SYSCALL:
        return "syscall";

    case KW_ENTRY:
        return "entry";

    case NKWS:
    default:
        return "unknown";
    }
}

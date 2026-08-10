#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "diagnostic.h"
#include "lexer.h"
#include "source.h"
#include "token.h"

int main(int argc, char **argv)
{
    if (argc < 2)
        Error("usage: %s <file>\n", argv[0]);

    Source source;
    if (!SourceLoad(argv[1], &source))
        Error("failed to load %s\n", argv[1]);

    TokenNode *node = Lex(&source);

    while (node)
    {
        usize len = TokenAsStrLen(&node->token);
        char *buf = alloc(len);
        TokenAsStr(&node->token, buf);
        printf("%s\n", buf);
        free(buf);
        node = node->next;
    }

    return 0;
}

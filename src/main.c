#include <stdio.h>
#include <stdlib.h>
#include "mem.h"
#include "diagnostic.h"
#include "lexer.h"
#include "source.h"
#include "token.h"
#include "stmt.h"
#include "parser.h"

int main(int argc, char **argv)
{
    if (argc < 2)
        Error("usage: %s <file>", argv[0]);

    Source source;
    if (!SourceLoad(argv[1], &source))
        Error("failed to load %s", argv[1]);

    TokenNode *tokens = Lex(&source);
    printf("Tokens:\n");
    for (TokenNode *tnode = tokens; tnode; tnode = tnode->next)
    {
        usize len = TokenAsStrLen(&tnode->token);
        char *buf = Alloc(len);
        TokenAsStr(&tnode->token, buf);
        printf("%s\n", buf);
        free(buf);
    }

    printf("\nAST:\n");
    StmtNode *stmts = Parse(&source, tokens);
    for (StmtNode *snode = stmts; snode; snode = snode->next)
    {
        usize len = StmtAsStrLen(&snode->stmt);
        char *buf = Alloc(len);
        StmtAsStr(&snode->stmt, buf);
        printf(" %s\n", buf);
        free(buf);
    }

    LexFree(&tokens);
    SourceFree(&source);

    return 0;
}

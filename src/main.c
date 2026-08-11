#include <stdio.h>
#include <sys/stat.h>
#include "mem.h"
#include "diagnostic.h"
#include "lexer.h"
#include "source.h"
#include "token.h"
#include "stmt.h"
#include "parser.h"
#include "cli.h"
#include "codegen.h"
#include "elfwriter.h"

int main(int argc, char **argv)
{
    CliOptions opts = ParseCli(argc, argv);

    Source source;
    if (!SourceLoad(opts.inputPath, &source))
        Error("failed to load %s", opts.inputPath);

    Diags diags;
    DiagsInit(&diags, 20);

    /* ---- STAGE_TOKENIZE ---- */
    TokenNode *tokens = Lex(&source, &diags);

    if (diags.len > 0)
    {
        DiagsReportAll(&diags, &source, "lexer error");
        DiagsFree(&diags);
        LexFree(&tokens);
        SourceFree(&source);
        CliOptionsFree(&opts);
        return 3;
    }

    if (opts.wantTokenize)
    {
        printf("Tokens:\n");
        for (TokenNode *tnode = tokens; tnode; tnode = tnode->next)
        {
            usize len = TokenAsStrLen(&tnode->token);
            char *buf = Alloc(len);
            TokenAsStr(&tnode->token, buf);
            printf("%s\n", buf);
            Free(buf);
        }
    }

    if (opts.furthest == STAGE_TOKENIZE)
    {
        LexFree(&tokens);
        SourceFree(&source);
        CliOptionsFree(&opts);
        return 0;
    }

    /* ---- STAGE_PARSE ---- */
    StmtNode *stmts = Parse(&source, tokens);
    if (opts.wantParse)
    {
        printf("\nAST:\n");
        for (StmtNode *snode = stmts; snode; snode = snode->next)
        {
            usize len = StmtAsStrLen(&snode->stmt);
            char *buf = Alloc(len);
            StmtAsStr(&snode->stmt, buf);
            printf(" %s\n", buf);
            Free(buf);
        }
    }

    if (opts.furthest == STAGE_PARSE)
    {
        ParseFree(&stmts);
        LexFree(&tokens);
        SourceFree(&source);
        CliOptionsFree(&opts);
        return 0;
    }

    /* ---- STAGE_COMPILE ---- */

    CodegenResult result = GenerateCode(&source, stmts);
    ByteBuf elf = BuildElf(&result);

    FILE *file = fopen(opts.outputPath, "wb");
    if (!file)
        Error("failed to open output file '%s'", opts.outputPath);

    if (fwrite(elf.data, 1, elf.len, file) != elf.len)
    {
        fclose(file);
        Error("failed to write output file '%s'", opts.outputPath);
    }

    if (fclose(file) != 0)
        Error("failed to close output file '%s'", opts.outputPath);

    if (chmod(opts.outputPath, 0755) != 0)
        Error("failed to make '%s' executable", opts.outputPath);

    ByteBufFree(&elf);
    ByteBufFree(&result.code);

    ParseFree(&stmts);
    LexFree(&tokens);
    DiagsFree(&diags);
    SourceFree(&source);
    CliOptionsFree(&opts);

    return 0;
}

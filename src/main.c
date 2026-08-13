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

#define DIAGS_CAP 20

int main(int argc, char **argv)
{
    CliOptions opts = ParseCli(argc, argv);

    Source source;
    if (!SourceLoad(opts.inputPath, &source))
        Error("failed to load %s", opts.inputPath);

    /* ---- STAGE_TOKENIZE ---- */
    Diags lexDiags;
    DiagsInit(&lexDiags, DIAGS_CAP);

    TokenNode *tokens = Lex(&source, &lexDiags);

    DiagsReportAll(&lexDiags, &source, "lexer");

    if (lexDiags.nerrs > 0)
    {
        DiagsFree(&lexDiags);
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
    Diags parseDiags;
    DiagsInit(&parseDiags, DIAGS_CAP);

    StmtNode *stmts = Parse(&source, tokens, &parseDiags);

    DiagsReportAll(&parseDiags, &source, "parser");

    if (parseDiags.nerrs > 0)
    {
        DiagsFree(&parseDiags);
        DiagsFree(&lexDiags);
        LexFree(&tokens);
        SourceFree(&source);
        CliOptionsFree(&opts);
        return 4;
    }

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
        DiagsFree(&parseDiags);
        DiagsFree(&lexDiags);
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
    DiagsFree(&parseDiags);
    DiagsFree(&lexDiags);
    SourceFree(&source);
    CliOptionsFree(&opts);

    return 0;
}

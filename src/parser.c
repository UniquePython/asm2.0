#include "parser.h"

#include "diagnostic.h"
#include "mem.h"
#include "suggest.h"
#include "syntax.h"

#include <string.h>

static Token ParserPeek(const Parser *p)
{
    if (!p->cursor)
        InternalError("parser cursor is NULL");
    return p->cursor->token;
}

static bool ParserCheckKeyword(const Parser *p, Keyword kw)
{
    Token tok = ParserPeek(p);
    return tok.tk == TK_KEYWORD && tok.as.keyword == kw;
}

static Token ParserAdvance(Parser *p)
{
    Token tok = ParserPeek(p);
    if (tok.tk != TK_EOF)
        p->cursor = p->cursor->next;
    return tok;
}

static Token ParserExpect(Parser *p, TokenKind kind, const char *syntax)
{
    Token tok = ParserPeek(p);

    if (tok.tk != kind)
    {
        DiagsPushFull(p->diags, tok.span, NULL, syntax,
                      "expected %s, found %s", TokenKindAsStr(kind), TokenKindAsStr(tok.tk));
        return NewEOFToken(tok.span.start);
    }

    return ParserAdvance(p);
}

static Token ParserExpectKeyword(Parser *p, Keyword kw, const char *syntax)
{
    Token tok = ParserPeek(p);

    if (tok.tk != TK_KEYWORD)
    {
        DiagsPushFull(p->diags, tok.span, NULL, syntax,
                      "expected keyword '%s', got %s", KeywordAsStr(kw), TokenKindAsStr(tok.tk));
        return NewEOFToken(tok.span.start);
    }

    if (tok.as.keyword != kw)
    {
        DiagsPushFull(p->diags, tok.span, NULL, syntax,
                      "expected keyword '%s', got keyword '%s'", KeywordAsStr(kw), KeywordAsStr(tok.as.keyword));
        return NewEOFToken(tok.span.start);
    }

    return ParserAdvance(p);
}

/*
 * label_decl ::= "label" IDENTIFIER ":"
 */
static Stmt ParseLabelDecl(Parser *p)
{
    Token labelTok = ParserExpectKeyword(p, KW_LABEL, LABEL_SYNTAX);
    Token nameTok = ParserExpect(p, TK_IDENTIFIER, LABEL_SYNTAX);
    Token colonTok = ParserExpect(p, TK_COLON, LABEL_SYNTAX);

    Span span = {
        .start = labelTok.span.start,
        .end = colonTok.span.end,
    };

    return NewLabelStmt(nameTok.as.identifier, nameTok.span, span);
}

/*
 * operand ::= IDENTIFIER | NUMBER
 */
static Operand ParseOperand(Parser *p, const char *syntax)
{
    Token tok = ParserPeek(p);

    switch (tok.tk)
    {
    case TK_IDENTIFIER:
        ParserAdvance(p);
        return NewIdentifierOperand(tok.as.identifier, tok.span);

    case TK_NUMBER:
        ParserAdvance(p);
        return NewNumberOperand(tok.as.number, tok.span);

    case TK_KEYWORD:
    case TK_COLON:
    case TK_SEMICOLON:
    case TK_EOF:
    default:
        DiagsPushFull(p->diags, tok.span, NULL, syntax,
                      "expected identifier or number, got %s", TokenKindAsStr(tok.tk));
        return NewNumberOperand(0, tok.span);
    }
}

/*
 * instruction ::= "move" operand "to" operand ";"
 */
static Stmt ParseMoveStmt(Parser *p)
{
    Token moveTok = ParserExpectKeyword(p, KW_MOVE, MOVE_SYNTAX);
    Operand src = ParseOperand(p, MOVE_SYNTAX);

    ParserExpectKeyword(p, KW_TO, MOVE_SYNTAX);

    Operand dest = ParseOperand(p, MOVE_SYNTAX);
    Token semicolonTok = ParserExpect(p, TK_SEMICOLON, MOVE_SYNTAX);

    Span span = {
        .start = moveTok.span.start,
        .end = semicolonTok.span.end,
    };

    return NewMoveStmt(src, dest, span);
}

/*
 * instruction ::= "syscall" ";"
 */
static Stmt ParseSyscallStmt(Parser *p)
{
    Token syscallTok = ParserExpectKeyword(p, KW_SYSCALL, SYSCALL_SYNTAX);
    Token semicolonTok = ParserExpect(p, TK_SEMICOLON, SYSCALL_SYNTAX);

    Span span = {
        .start = syscallTok.span.start,
        .end = semicolonTok.span.end,
    };

    return NewSyscallStmt(span);
}

/*
 * entry_decl  ::= "entry" IDENTIFIER ";"
 */
static Stmt ParseEntryStmt(Parser *p)
{
    Token entryTok = ParserExpectKeyword(p, KW_ENTRY, ENTRY_SYNTAX);
    Token labelTok = ParserExpect(p, TK_IDENTIFIER, ENTRY_SYNTAX);
    Token semicolonTok = ParserExpect(p, TK_SEMICOLON, ENTRY_SYNTAX);

    Span span = {
        .start = entryTok.span.start,
        .end = semicolonTok.span.end,
    };

    return NewEntryStmt(labelTok.as.identifier, labelTok.span, span);
}

static Stmt ParseStmt(Parser *p)
{
    if (ParserCheckKeyword(p, KW_LABEL))
        return ParseLabelDecl(p);

    if (ParserCheckKeyword(p, KW_MOVE))
        return ParseMoveStmt(p);

    if (ParserCheckKeyword(p, KW_SYSCALL))
        return ParseSyscallStmt(p);

    if (ParserCheckKeyword(p, KW_ENTRY))
        return ParseEntryStmt(p);

    Token tok = ParserPeek(p);
    char *help = NULL;
    if (tok.tk == TK_IDENTIFIER)
    {
        usize len = strlen(tok.as.identifier);
        usize maxDist = len / 3 < 1 ? 1 : len / 3;
        const char *suggestion = SuggestClosest(tok.as.identifier, stmtStartingKeywords, 4, maxDist);
        if (suggestion)
            help = HelpMessage("did you mean '%s'?", suggestion);
    }

    DiagsPushFull(p->diags, tok.span, help, NULL, "expected statement, got %s", TokenKindAsStr(tok.tk));

    return NewSyscallStmt(tok.span); /* dummy -- discarded by caller, see Parse() */
}

static void ParserSynchronize(Parser *p)
{
    for (;;)
    {
        Token tok = ParserPeek(p);

        if (tok.tk == TK_EOF)
            return;

        if (tok.tk == TK_KEYWORD &&
            (tok.as.keyword == KW_LABEL ||
             tok.as.keyword == KW_MOVE ||
             tok.as.keyword == KW_SYSCALL ||
             tok.as.keyword == KW_ENTRY))
            return;

        ParserAdvance(p);

        if (tok.tk == TK_SEMICOLON)
            return; /* just consumed a ';' -- next token is a fresh start */
    }
}

/*
 * Post-pass: warn about labels with no statements between them and
 * whatever comes next (the next label, or EOF). `entry` doesn't count
 * as body content -- it's a standalone declaration that can legally
 * appear anywhere and isn't part of any label's body, so a label
 * immediately followed only by `entry` statement(s) and then another
 * label (or EOF) is still considered empty.
 *
 * Runs over the flat, already-parsed statement list rather than
 * during parsing itself, since "empty body" is a property of what
 * comes *between* two statements, not of any single statement in
 * isolation -- the parser doesn't have that context mid-parse, but by
 * the time Parse() is about to return, the whole list is available.
 *
 * This is a warning, not an error: DiagsPushWarningFull does not
 * trigger ParserSynchronize and does not stop `stmts` from being
 * returned and used -- see DiagsPushImpl's severity handling and
 * main.c's `nerrs`-gated (not `len`-gated) abort check.
 */
static void CheckEmptyLabelBodies(const StmtNode *stmts, Diags *diags)
{
    for (const StmtNode *node = stmts; node; node = node->next)
    {
        if (node->stmt.kind != STMT_LABEL)
            continue;

        const StmtNode *next = node->next;
        while (next && next->stmt.kind == STMT_ENTRY)
            next = next->next;

        bool empty = (next == NULL) || (next->stmt.kind == STMT_LABEL);

        if (empty)
            DiagsPushWarningFull(diags, node->stmt.span,
                                 HelpMessage("add a statement below it, e.g. 'syscall;', or remove the label if it's unused"),
                                 NULL,
                                 "label '%s' has an empty body", node->stmt.as.label.name);
    }
}

/*
 * program ::= statement*
 */
StmtNode *Parse(const Source *source, TokenNode *tokens, Diags *diags)
{
    Parser p = {
        .cursor = tokens,
        .source = source,
        .diags = diags,
    };

    StmtNode *head = NULL;
    StmtNode *tail = NULL;

    while (ParserPeek(&p).tk != TK_EOF)
    {
        usize errsBefore = diags->nerrs;

        Stmt stmt = ParseStmt(&p);

        if (diags->nerrs != errsBefore)
        {
            ParserSynchronize(&p);
            continue; /* discard `stmt` -- it may hold dummy/placeholder data */
        }

        StmtNode *node = Alloc(sizeof(*node));

        *node = (StmtNode){
            .stmt = stmt,
            .next = NULL,
        };

        if (tail)
            tail->next = node;
        else
            head = node;

        tail = node;
    }

    CheckEmptyLabelBodies(head, diags);

    return head;
}

void ParseFree(StmtNode **head)
{
    if (!head)
        return;

    StmtNode *node = *head;
    while (node)
    {
        StmtNode *next = node->next;
        Free(node);
        node = next;
    }

    *head = NULL;
}

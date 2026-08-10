#include "parser.h"

#include "diagnostic.h"
#include "mem.h"

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

static Token ParserExpect(Parser *p, TokenKind kind)
{
    Token tok = ParserPeek(p);
    if (tok.tk != kind)
        ParseError(p->source, tok.span, "expected %s, found %s", TokenKindAsStr(kind), TokenKindAsStr(tok.tk));
    return ParserAdvance(p);
}

static Token ParserExpectKeyword(Parser *p, Keyword kw)
{
    Token tok = ParserPeek(p);

    if (tok.tk != TK_KEYWORD)
        ParseError(p->source, tok.span, "expected keyword '%s', got %s", KeywordAsStr(kw), TokenKindAsStr(tok.tk));

    if (tok.as.keyword != kw)
        ParseError(p->source, tok.span, "expected keyword '%s', got keyword '%s'", KeywordAsStr(kw), KeywordAsStr(tok.as.keyword));

    return ParserAdvance(p);
}

/*
 * label_decl ::= "label" IDENTIFIER ":"
 */
static Stmt ParseLabelDecl(Parser *p)
{
    Token labelTok = ParserExpectKeyword(p, KW_LABEL);
    Token nameTok = ParserExpect(p, TK_IDENTIFIER);
    Token colonTok = ParserExpect(p, TK_COLON);

    Span span = {
        .start = labelTok.span.start,
        .end = colonTok.span.end,
    };

    return NewLabelStmt(nameTok.as.identifier, nameTok.span, span);
}

/*
 * operand ::= IDENTIFIER | NUMBER
 */
static Operand ParseOperand(Parser *p)
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
        ParseError(p->source, tok.span, "expected identifier or number, got %s", TokenKindAsStr(tok.tk));
    }

    /* Unreachable: ParseError does not return. */
    return NewNumberOperand(0, (Span){0});
}

/*
 * instruction ::= "move" operand "to" operand ";"
 */
static Stmt ParseMoveStmt(Parser *p)
{
    Token moveTok = ParserExpectKeyword(p, KW_MOVE);
    Operand src = ParseOperand(p);

    ParserExpectKeyword(p, KW_TO);

    Operand dest = ParseOperand(p);
    Token semicolonTok = ParserExpect(p, TK_SEMICOLON);

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
    Token syscallTok = ParserExpectKeyword(p, KW_SYSCALL);
    Token semicolonTok = ParserExpect(p, TK_SEMICOLON);

    Span span = {
        .start = syscallTok.span.start,
        .end = semicolonTok.span.end,
    };

    return NewSyscallStmt(span);
}

static Stmt ParseStmt(Parser *p)
{
    if (ParserCheckKeyword(p, KW_LABEL))
        return ParseLabelDecl(p);

    if (ParserCheckKeyword(p, KW_MOVE))
        return ParseMoveStmt(p);

    if (ParserCheckKeyword(p, KW_SYSCALL))
        return ParseSyscallStmt(p);

    Token tok = ParserPeek(p);
    ParseError(p->source, tok.span, "expected statement, got %s", TokenKindAsStr(tok.tk));

    /* Unreachable: ParseError does not return. */
    return ParseLabelDecl(p);
}

/*
 * program ::= statement*
 */
StmtNode *Parse(const Source *source, TokenNode *tokens)
{
    Parser p = {
        .cursor = tokens,
        .source = source,
    };

    StmtNode *head = NULL;
    StmtNode *tail = NULL;

    while (ParserPeek(&p).tk != TK_EOF)
    {
        Stmt stmt = ParseStmt(&p);

        StmtNode *node = alloc(sizeof(*node));

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

    return head;
}

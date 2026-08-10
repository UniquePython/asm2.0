#include "lexer.h"

#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "diagnostic.h"
#include "keyword.h"

static u32 SkipWhitespace(const Source *source, u32 pos)
{
    while (pos < source->len)
    {
        switch (source->data[pos])
        {
        case ' ':
        case '\t':
        case '\n':
        case '\r':
        case '\v':
        case '\f':
            pos++;
            break;

        default:
            return pos;
        }
    }

    return pos;
}

static u32 SkipComment(const Source *source, u32 pos)
{
    if (pos >= source->len || source->data[pos] != '#')
        return pos;

    while (pos < source->len && source->data[pos] != '\n')
        pos++;

    return pos;
}

static bool IsIdentStart(char ch)
{
    return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

static bool IsIdentContinue(char ch)
{
    return IsIdentStart(ch) || (ch >= '0' && ch <= '9');
}

static Token LexIdentifierOrKeyword(const Source *source, u32 *pos)
{
    u32 start = *pos;

    while (*pos < source->len && IsIdentContinue(source->data[*pos]))
        (*pos)++;

    Span span = {
        .start = start,
        .end = *pos,
    };

    Keyword kw;

    if (StrToKeyword(source->data + start, SpanLength(span), &kw))
        return NewKeywordToken(kw, span);

    usize len = SpanLength(span);
    char *copy = alloc(len + 1);

    memcpy(copy, source->data + start, len);
    copy[len] = '\0';

    return NewIdentifierToken(copy, span);
}

static Token LexNumber(const Source *source, u32 *pos)
{
    u32 start = *pos;
    u64 value = 0;

    while (*pos < source->len && source->data[*pos] >= '0' && source->data[*pos] <= '9')
    {
        u64 digit = (u64)(source->data[*pos] - '0');

        if (value > (UINT64_MAX - digit) / 10)
            LexError(source, (Span){.start = start, .end = *pos + 1}, "integer literal is too large");

        value = value * 10 + digit;
        (*pos)++;
    }

    Span span = {
        .start = start,
        .end = *pos,
    };

    return NewNumberToken(value, span);
}

TokenNode *Lex(const Source *source)
{
    u32 pos = 0;
    TokenNode *head = NULL;
    TokenNode *tail = NULL;

    for (;;)
    {
        u32 prevPos;
        do
        {
            prevPos = pos;
            pos = SkipWhitespace(source, pos);
            pos = SkipComment(source, pos);
        } while (pos != prevPos);

        Token token;

        if (pos >= source->len)
            token = NewEOFToken(pos);
        else
        {
            char ch = source->data[pos];

            if (IsIdentStart(ch))
                token = LexIdentifierOrKeyword(source, &pos);
            else if (ch >= '0' && ch <= '9')
                token = LexNumber(source, &pos);
            else if (ch == ':')
            {
                token = NewSimpleToken(TK_COLON, (Span){.start = pos, .end = pos + 1});
                pos += 1;
            }
            else if (ch == ';')
            {
                token = NewSimpleToken(TK_SEMICOLON, (Span){.start = pos, .end = pos + 1});
                pos += 1;
            }
            else
            {
                Span badSpan = {.start = pos, .end = pos + 1};
                LexError(source, badSpan, "unexpected character '%c'", ch);
                /* unreachable: LexError never returns */
                token = NewEOFToken(pos);
            }
        }

        TokenNode *node = alloc(sizeof(*node));
        *node = (TokenNode){
            .token = token,
            .next = NULL,
        };
        if (tail)
        {
            tail->next = node;
            tail = node;
        }
        else
        {
            head = node;
            tail = node;
        }

        if (token.tk == TK_EOF)
            break;
    }

    return head;
}

void LexFree(TokenNode **head)
{
    if (!head)
        return;

    TokenNode *node = *head;

    while (node)
    {
        TokenNode *next = node->next;

        if (node->token.tk == TK_IDENTIFIER)
            free(node->token.as.identifier);

        free(node);
        node = next;
    }

    *head = NULL;
}

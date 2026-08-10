#include "operand.h"

#include <stdio.h>

Operand NewIdentifierOperand(const char *identifier, Span span)
{
    return (Operand){
        .kind = OPERAND_IDENTIFIER,
        .span = span,
        .as.identifier = identifier,
    };
}

Operand NewNumberOperand(u64 number, Span span)
{
    return (Operand){
        .kind = OPERAND_NUMBER,
        .span = span,
        .as.number = number,
    };
}

#define SPAN_FMT " @" U32_FMT ":" U32_FMT
#define SPAN_ARG(span) (span).start, (span).end
static int OperandFormat(const Operand *operand, char *buffer, usize len)
{
    switch (operand->kind)
    {
    case OPERAND_IDENTIFIER:
        return snprintf(buffer, len, "Operand identifier(\"%s\")" SPAN_FMT, operand->as.identifier, SPAN_ARG(operand->span));

    case OPERAND_NUMBER:
        return snprintf(buffer, len, "Operand number(" U64_FMT ")" SPAN_FMT, operand->as.number, SPAN_ARG(operand->span));

    default:
        return snprintf(buffer, len, "Operand %s" SPAN_FMT, "unknown", SPAN_ARG(operand->span));
    }
}
#undef SPAN_ARG
#undef SPAN_FMT

usize OperandAsStrLen(const Operand *operand)
{
    int n = OperandFormat(operand, NULL, 0);
    if (n < 0)
        return 0;
    return (usize)n + 1;
}

bool OperandAsStr(const Operand *operand, char *buffer)
{
    usize len = OperandAsStrLen(operand);
    if (len == 0)
        return false;
    return OperandFormat(operand, buffer, len) > 0 ? true : false;
}

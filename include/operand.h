#ifndef OPERAND_H_
#define OPERAND_H_

#include "types.h"
#include "span.h"
#include "operandkind.h"

typedef struct operand_t
{
    OperandKind kind;
    Span span;

    union
    {
        const char *identifier;
        u64 number;
    } as;
} Operand;

Operand NewIdentifierOperand(const char *identifier, Span span);
Operand NewNumberOperand(u64 number, Span span);

usize OperandAsStrLen(const Operand *operand);
bool OperandAsStr(const Operand *operand, char *buffer);

#endif /* OPERAND_H_ */

#ifndef OPERANDKIND_H_
#define OPERANDKIND_H_

typedef enum operand_kind_t
{
    OPERAND_IDENTIFIER,
    OPERAND_NUMBER,
} OperandKind;

const char *OperandKindAsStr(OperandKind kind);

#endif /* OPERANDKIND_H_ */

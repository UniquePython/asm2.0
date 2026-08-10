#include "operandkind.h"

const char *OperandKindAsStr(OperandKind kind)
{
    switch (kind)
    {
    case OPERAND_IDENTIFIER:
        return "identifier";

    case OPERAND_NUMBER:
        return "number";

    default:
        return "unknown";
    }
}

#include "codegen.h"

#include <string.h>

#include "diagnostic.h"
#include "mem.h"

Register ResolveRegisterOperand(const Source *source, const Operand *operand)
{
    if (operand->kind != OPERAND_IDENTIFIER)
        CodegenError(source, operand->span, "expected a register, got a number");

    Register reg;
    if (!StrToRegister(operand->as.identifier, strlen(operand->as.identifier), &reg))
        CodegenError(source, operand->span, "unknown register '%s'", operand->as.identifier);

    return reg;
}

/*
 * label ::= no bytes -- nothing to emit yet, addresses aren't resolved
 * or referenced anywhere in the language so far.
 */
static Encoded EncodeLabelStmt(void)
{
    return (Encoded){
        .bytes = NULL,
        .len = 0,
    };
}

/*
 * syscall -> 0F 05, fixed, no operands.
 */
static Encoded EncodeSyscallStmt(void)
{
    u8 *bytes = alloc(2);
    bytes[0] = 0x0F;
    bytes[1] = 0x05;

    return (Encoded){
        .bytes = bytes,
        .len = 2,
    };
}

/*
 * move <number> to <register> -> mov reg32, imm32
 * Opcode: B8 + register index, followed by the 4-byte little-endian
 * immediate. Register-to-register moves and immediates that don't fit
 * in 32 bits are not supported yet.
 */
static Encoded EncodeMoveStmt(const Source *source, const Stmt *stmt)
{
    const Operand *src = &stmt->as.move.src;
    const Operand *dest = &stmt->as.move.dest;

    if (src->kind != OPERAND_NUMBER)
        CodegenError(source, src->span, "only numeric sources are supported for 'move' right now");

    Register destReg = ResolveRegisterOperand(source, dest);

    if (src->as.number > UINT32_MAX)
        CodegenError(source, src->span, "immediate value is too large to fit in 32 bits");

    u8 *bytes = alloc(5);
    bytes[0] = (u8)(0xB8 + (u8)destReg);

    u32 imm = (u32)src->as.number;
    bytes[1] = (u8)(imm & 0xFF);
    bytes[2] = (u8)((imm >> 8) & 0xFF);
    bytes[3] = (u8)((imm >> 16) & 0xFF);
    bytes[4] = (u8)((imm >> 24) & 0xFF);

    return (Encoded){
        .bytes = bytes,
        .len = 5,
    };
}

Encoded EncodeStmt(const Source *source, const Stmt *stmt)
{
    switch (stmt->kind)
    {
    case STMT_LABEL:
        return EncodeLabelStmt();

    case STMT_SYSCALL:
        return EncodeSyscallStmt();

    case STMT_MOVE:
        return EncodeMoveStmt(source, stmt);

    default:
        InternalError("unknown statement kind in EncodeStmt");
    }

    /* Unreachable: InternalError does not return. */
    return (Encoded){.bytes = NULL, .len = 0};
}

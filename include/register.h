#ifndef REGISTER_H_
#define REGISTER_H_

#include "types.h"

typedef enum register_t
{
    REG_EAX,
    REG_ECX,
    REG_EDX,
    REG_EBX,
    REG_ESP,
    REG_EBP,
    REG_ESI,
    REG_EDI,
    REG_R8D,
    REG_R9D,
    REG_R10D,
    REG_R11D,
    REG_R12D,
    REG_R13D,
    REG_R14D,
    REG_R15D,
    NREGS,
} Register;

extern const char *const registers[NREGS];

bool StrToRegister(const char *str, usize len, Register *out);

const char *RegisterAsStr(Register reg);

/*
 * The low 3 bits of a register's index -- the value that actually
 * fits in a ModRM Reg/R/M field or an opcode's embedded +rd slot.
 * For eax..edi this is just the register's own index (0-7). For
 * r8d..r15d it's the index with the high bit masked off (e.g. r9d,
 * index 9, has low bits 1 -- the same bit pattern as ecx). The
 * missing high bit must be supplied separately via REX -- see
 * RegisterNeedsRex.
 */
u8 RegisterLowBits(Register reg);

/*
 * Whether selecting this register requires a REX prefix to supply
 * its high bit (true for r8d..r15d, false for the original eight).
 */
bool RegisterNeedsRex(Register reg);

#endif /* REGISTER_H_ */

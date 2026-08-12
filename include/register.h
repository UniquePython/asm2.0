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
    NREGS,
} Register;

extern const char *const registers[NREGS];

bool StrToRegister(const char *str, usize len, Register *out);

const char *RegisterAsStr(Register reg);

#endif /* REGISTER_H_ */

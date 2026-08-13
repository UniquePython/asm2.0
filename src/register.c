#include "register.h"

#include <string.h>
#include <assert.h>

const char *const registers[] = {
    "eax",
    "ecx",
    "edx",
    "ebx",
    "esp",
    "ebp",
    "esi",
    "edi",
    "r8d",
    "r9d",
    "r10d",
    "r11d",
    "r12d",
    "r13d",
    "r14d",
    "r15d",
};

static_assert(sizeof(registers) / sizeof(registers[0]) == NREGS, "registers[] must have exactly NREGS entries, in enum order");

bool StrToRegister(const char *str, usize len, Register *out)
{
    for (usize idx = 0; idx < NREGS; idx++)
    {
        const char *reg = registers[idx];
        if (strlen(reg) == len && strncmp(reg, str, len) == 0)
        {
            *out = (Register)idx;
            return true;
        }
    }

    return false;
}

const char *RegisterAsStr(Register reg)
{
    switch (reg)
    {
    case REG_EAX:
        return "eax";

    case REG_ECX:
        return "ecx";

    case REG_EDX:
        return "edx";

    case REG_EBX:
        return "ebx";

    case REG_ESP:
        return "esp";

    case REG_EBP:
        return "ebp";

    case REG_ESI:
        return "esi";

    case REG_EDI:
        return "edi";

    case REG_R8D:
        return "r8d";

    case REG_R9D:
        return "r9d";

    case REG_R10D:
        return "r10d";

    case REG_R11D:
        return "r11d";

    case REG_R12D:
        return "r12d";

    case REG_R13D:
        return "r13d";

    case REG_R14D:
        return "r14d";

    case REG_R15D:
        return "r15d";

    case NREGS:
    default:
        return "unknown";
    }
}

u8 RegisterLowBits(Register reg)
{
    return (u8)reg & 0x7u;
}

bool RegisterNeedsRex(Register reg)
{
    return ((u8)reg & 0x8u) != 0;
}

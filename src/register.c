#include "register.h"

#include <string.h>

static const char *registers[] = {
    "eax",
    "ecx",
    "edx",
    "ebx",
    "esp",
    "ebp",
    "esi",
    "edi",
};

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

    case NREGS:
    default:
        return "unknown";
    }
}

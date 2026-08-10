#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "types.h"
#include "source.h"
#include "operand.h"
#include "stmt.h"
#include "register.h"

typedef struct encoded_t
{
    u8 *bytes;
    usize len;
} Encoded;

Register ResolveRegisterOperand(const Source *source, const Operand *operand);
Encoded EncodeStmt(const Source *source, const Stmt *stmt);

#endif /* CODEGEN_H_ */

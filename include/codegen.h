#ifndef CODEGEN_H_
#define CODEGEN_H_

#include "types.h"
#include "source.h"
#include "operand.h"
#include "stmt.h"
#include "parser.h"
#include "bytebuf.h"
#include "register.h"

typedef struct encoded_t
{
    u8 *bytes;
    usize len;
} Encoded;

Register ResolveRegisterOperand(const Source *source, const Operand *operand);
Encoded EncodeStmt(const Source *source, const Stmt *stmt);

typedef struct codegen_result_t
{
    ByteBuf code;
    u64 entryOffset;
} CodegenResult;

CodegenResult GenerateCode(const Source *source, const StmtNode *stmts);

#endif /* CODEGEN_H_ */

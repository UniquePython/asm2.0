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

// Nothing to emit
static Encoded EncodeEntryStmt(void)
{
    return (Encoded){
        .bytes = NULL,
        .len = 0,
    };
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
    u8 *bytes = Alloc(2);
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

    u8 *bytes = Alloc(5);
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

typedef struct label_entry_t
{
    const char *name;
    usize offset;
} LabelEntry;

typedef struct label_table_t
{
    LabelEntry *entries;
    usize len;
} LabelTable;

static usize CountLabels(const StmtNode *stmts)
{
    usize count = 0;

    for (const StmtNode *node = stmts; node; node = node->next)
        if (node->stmt.kind == STMT_LABEL)
            count++;

    return count;
}

static bool LabelTableFind(const LabelTable *table, const char *name, usize *outOffset)
{
    for (usize i = 0; i < table->len; i++)
    {
        if (strcmp(table->entries[i].name, name) == 0)
        {
            *outOffset = table->entries[i].offset;
            return true;
        }
    }

    return false;
}

static void LabelTableFree(LabelTable *table)
{
    if (!table)
        return;

    Free(table->entries);

    table->entries = NULL;
    table->len = 0;
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

    case STMT_ENTRY:
        return EncodeEntryStmt();

    default:
        InternalError("unknown statement kind in EncodeStmt");
    }

    /* Unreachable: InternalError does not return. */
    return (Encoded){.bytes = NULL, .len = 0};
}

CodegenResult GenerateCode(const Source *source, const StmtNode *stmts)
{
    usize labelCount = CountLabels(stmts);

    LabelTable table = {
        .entries = labelCount > 0 ? Alloc(labelCount * sizeof(LabelEntry)) : NULL,
        .len = 0,
    };

    ByteBuf code;
    ByteBufInit(&code);

    /* First pass: encode code and build label table. */
    for (const StmtNode *node = stmts; node; node = node->next)
    {
        const Stmt *stmt = &node->stmt;

        if (stmt->kind == STMT_LABEL)
        {
            usize existingOffset;
            if (LabelTableFind(&table, stmt->as.label.name, &existingOffset))
                CodegenError(source, stmt->span, "duplicate label '%s'", stmt->as.label.name);

            table.entries[table.len++] = (LabelEntry){
                .name = stmt->as.label.name,
                .offset = code.len,
            };
        }

        if (stmt->kind == STMT_ENTRY)
            continue;

        Encoded encoded = EncodeStmt(source, stmt);

        for (usize i = 0; i < encoded.len; i++)
            ByteBufWriteU8(&code, encoded.bytes[i]);

        Free(encoded.bytes);
    }

    /* Second pass: resolve the entry label. */
    const Stmt *entry = NULL;
    u64 entryOffset = 0;

    for (const StmtNode *node = stmts; node; node = node->next)
    {
        if (node->stmt.kind != STMT_ENTRY)
            continue;

        if (entry)
            CodegenError(source, node->stmt.span, "multiple entry statements");

        entry = &node->stmt;
    }

    if (!entry)
        CodegenError(source, (Span){.start = 0, .end = 0}, "no entry statement");

    if (!LabelTableFind(&table, entry->as.entry.label, &entryOffset))
        CodegenError(source, entry->span, "entry refers to undefined label '%s'", entry->as.entry.label);

    LabelTableFree(&table);

    return (CodegenResult){
        .code = code,
        .entryOffset = entryOffset,
    };
}

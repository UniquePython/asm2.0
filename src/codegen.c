#include "codegen.h"

#include <string.h>

#include "diagnostic.h"
#include "mem.h"
#include "syntax.h"
#include "suggest.h"

Register ResolveRegisterOperand(const Source *source, const Operand *operand)
{
    if (operand->kind != OPERAND_IDENTIFIER)
        CodegenErrorFull(source, operand->span, NULL, MOVE_SYNTAX, "expected a register, got a number");

    Register reg;
    if (!StrToRegister(operand->as.identifier, strlen(operand->as.identifier), &reg))
    {
        usize len = strlen(operand->as.identifier);
        usize maxDist = len / 3 < 1 ? 1 : len / 3;
        const char *suggestion = SuggestClosest(operand->as.identifier, registers, NREGS, maxDist);

        CodegenErrorFull(source, operand->span,
                         suggestion ? HelpMessage("did you mean '%s'?", suggestion) : NULL,
                         MOVE_SYNTAX,
                         "unknown register '%s'", operand->as.identifier);
    }

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
 * ModRM byte, Mod=11 (register-direct) only -- no memory operands yet.
 *
 *   7 6 5 4 3 2 1 0
 *  +---+-----+-----+
 *  |Mod| Reg | R/M |
 *  +---+-----+-----+
 *
 * reg and rm are raw 3-bit fields (0-7) -- callers must pass the
 * result of RegisterLowBits, not a raw Register, since a Register can
 * now be as large as 15 (r8d..r15d) and would silently corrupt
 * adjacent bits otherwise. The missing high bit for such registers is
 * supplied separately via REX -- see BuildRex. Caller decides which
 * logical operand (src/dest) goes in which field -- see the opcode
 * comment in EncodeMoveRegToRegStmt.
 */
static u8 BuildModRM(u8 reg, u8 rm)
{
    return (u8)(0xC0 | (reg << 3) | rm);
    //          ^^^^ Mod=11, fixed
}

/*
 * REX prefix, W=0 (32-bit operand size) and X=0 (no SIB/memory
 * addressing yet) -- only R and B are ever set for now.
 *
 *   7 6 5 4   3   2   1   0
 *  +-------+---+---+---+---+
 *  | 0100  | W | R | X | B |
 *  +-------+---+---+---+---+
 *
 * reg is the register occupying ModRM's Reg field (or, for the +rd
 * opcode-embedded-register form, N/A -- see BuildRexForOpcodeReg);
 * rm is the register occupying ModRM's R/M field. Only call this when
 * at least one of them actually needs it (RegisterNeedsRex) -- when
 * neither does, no REX byte should be emitted at all.
 */
static u8 BuildRex(Register reg, Register rm)
{
    u8 r = RegisterNeedsRex(reg) ? 1 : 0;
    u8 b = RegisterNeedsRex(rm) ? 1 : 0;

    return (u8)(0x40 | (r << 2) | b);
}

/*
 * move <register> to <register> -> mov r32, r/m32
 * Opcode: 8B /r
 *   ModRM.Reg = dest, ModRM.R/M = src
 *
 * (Chosen so that a future 'move [addr] to reg' -- a load -- reuses
 * this same opcode/field assignment, with R/M becoming a memory
 * operand instead of a register. The reverse direction, storing a
 * register into memory, will need opcode 89 /r later.)
 *
 * When either operand is one of r8d..r15d, a REX prefix (REX.R for
 * the dest/Reg field, REX.B for the src/R/M field) is prepended.
 * ModRM's fields always carry only the low 3 bits of each register
 * either way -- REX supplies the missing high bit, it doesn't change
 * what ModRM itself contains.
 */
static Encoded EncodeMoveRegToRegStmt(const Source *source, const Operand *src, const Operand *dest)
{
    Register srcReg = ResolveRegisterOperand(source, src);
    Register destReg = ResolveRegisterOperand(source, dest);

    u8 modrm = BuildModRM(RegisterLowBits(destReg), RegisterLowBits(srcReg));

    if (RegisterNeedsRex(srcReg) || RegisterNeedsRex(destReg))
    {
        u8 *bytes = Alloc(3);
        bytes[0] = BuildRex(destReg, srcReg);
        bytes[1] = 0x8B;
        bytes[2] = modrm;

        return (Encoded){
            .bytes = bytes,
            .len = 3,
        };
    }

    u8 *bytes = Alloc(2);
    bytes[0] = 0x8B;
    bytes[1] = modrm;

    return (Encoded){
        .bytes = bytes,
        .len = 2,
    };
}

/*
 * move <number> to <register> -> mov reg32, imm32
 * Opcode: B8 + register index, followed by the 4-byte little-endian
 * immediate. Immediates that don't fit in 32 bits are not supported
 * yet.
 *
 * The destination register is embedded directly in the opcode's low
 * 3 bits (a "+rd" encoding -- there is no ModRM byte here at all).
 * When the destination is one of r8d..r15d, its high bit doesn't fit
 * in the opcode byte, so a REX prefix (REX.B) is prepended to supply
 * it -- the same role REX.B plays for ModRM's R/M field, just applied
 * to this opcode-embedded register instead.
 */
static Encoded EncodeMoveStmt(const Source *source, const Stmt *stmt)
{
    const Operand *src = &stmt->as.move.src;
    const Operand *dest = &stmt->as.move.dest;

    if (src->kind == OPERAND_IDENTIFIER)
        return EncodeMoveRegToRegStmt(source, src, dest);

    Register destReg = ResolveRegisterOperand(source, dest);

    if (src->as.number > UINT32_MAX)
        CodegenErrorFull(source, src->span,
                         HelpMessage("the largest value 'move' currently supports is %u", UINT32_MAX),
                         MOVE_SYNTAX, "immediate value is too large to fit in 32 bits");

    u32 imm = (u32)src->as.number;
    u8 opcode = (u8)(0xB8 + RegisterLowBits(destReg));

    if (RegisterNeedsRex(destReg))
    {
        u8 *bytes = Alloc(6);
        bytes[0] = 0x40 | 0x1; // REX.B -- extends the opcode-embedded register
        bytes[1] = opcode;
        bytes[2] = (u8)(imm & 0xFF);
        bytes[3] = (u8)((imm >> 8) & 0xFF);
        bytes[4] = (u8)((imm >> 16) & 0xFF);
        bytes[5] = (u8)((imm >> 24) & 0xFF);

        return (Encoded){
            .bytes = bytes,
            .len = 6,
        };
    }

    u8 *bytes = Alloc(5);
    bytes[0] = opcode;
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
                CodegenErrorFull(source, stmt->span,
                                 HelpMessage("each label name must be unique -- pick a different name for one of them"),
                                 LABEL_SYNTAX,
                                 "duplicate label '%s'", stmt->as.label.name);

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
            CodegenErrorFull(source, node->stmt.span,
                             HelpMessage("a program can only have one 'entry' statement -- remove one of them"),
                             ENTRY_SYNTAX, "multiple entry statements");

        entry = &node->stmt;
    }

    if (!entry)
        CodegenErrorFull(source, (Span){.start = 0, .end = 0},
                         HelpMessage("every program needs exactly one 'entry' statement naming where execution starts"),
                         ENTRY_SYNTAX, "no entry statement");

    if (!LabelTableFind(&table, entry->as.entry.label, &entryOffset))
    {
        char *help;
        if (table.len == 0)
            help = HelpMessage("this program has no labels defined. Try defining a label called '%s'", entry->as.entry.label);
        else
        {
            const char **labelNames = Alloc(table.len * sizeof(*labelNames));
            for (usize i = 0; i < table.len; i++)
                labelNames[i] = table.entries[i].name;

            usize len = strlen(entry->as.entry.label);
            usize maxDist = len / 3 < 1 ? 1 : len / 3;
            const char *suggestion = SuggestClosest(entry->as.entry.label, labelNames, table.len, maxDist);
            help = suggestion ? HelpMessage("did you mean '%s'?", suggestion) : NULL;
            Free(labelNames);
        }
        CodegenErrorFull(source, entry->span, help, LABEL_SYNTAX, "entry refers to undefined label '%s'", entry->as.entry.label);
    }

    LabelTableFree(&table);

    return (CodegenResult){
        .code = code,
        .entryOffset = entryOffset,
    };
}

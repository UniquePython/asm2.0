#include "elfwriter.h"

#include <elf.h>

/*
 * Toolchain-fixed layout constants. These are NOT configurable -- every
 * program this compiler emits uses exactly one PT_LOAD segment, loaded
 * whole (headers included) at a single fixed base address.
 */
#define BASE_ADDR 0x400000ULL
#define EHDR_SIZE 64
#define PHDR_SIZE 56

ByteBuf BuildElf(const CodegenResult *result)
{
    ByteBuf out;
    ByteBufInit(&out);

    /*
     * e_entry: the virtual address execution actually starts at.
     * result->entryOffset is "bytes into the code region" (see
     * GenerateCode) -- code always sits right after Ehdr+Phdr in this
     * layout, so the absolute address is base + headers + offset.
     */
    u64 entry = BASE_ADDR + EHDR_SIZE + PHDR_SIZE + result->entryOffset;

    /*
     * p_filesz / p_memsz: this PT_LOAD segment covers the WHOLE file
     * (headers included), copied byte-for-byte into memory with no
     * extra zeroed tail -- so filesz == memsz == everything we're
     * about to write.
     */
    u64 totalSize = (u64)EHDR_SIZE + (u64)PHDR_SIZE + (u64)result->code.len;

    /* ================= Elf64_Ehdr ================= */

    /* --- e_ident[0..15] --- */

    ByteBufWriteU8(&out, 0x7f);
    ByteBufWriteU8(&out, 'E');
    ByteBufWriteU8(&out, 'L');
    ByteBufWriteU8(&out, 'F');

    ByteBufWriteU8(&out, ELFCLASS64);
    ByteBufWriteU8(&out, ELFDATA2LSB);
    ByteBufWriteU8(&out, EV_CURRENT);
    ByteBufWriteU8(&out, ELFOSABI_NONE);
    ByteBufWriteU8(&out, 0);

    for (usize i = 0; i < 7; i++)
        ByteBufWriteU8(&out, 0);

    /* --- rest of Ehdr --- */

    ByteBufWriteU16LE(&out, ET_EXEC);
    ByteBufWriteU16LE(&out, EM_X86_64);
    ByteBufWriteU32LE(&out, EV_CURRENT);
    ByteBufWriteU64LE(&out, entry);
    ByteBufWriteU64LE(&out, EHDR_SIZE);
    ByteBufWriteU64LE(&out, 0); /* e_shoff */
    ByteBufWriteU32LE(&out, 0); /* e_flags */
    ByteBufWriteU16LE(&out, EHDR_SIZE);
    ByteBufWriteU16LE(&out, PHDR_SIZE);
    ByteBufWriteU16LE(&out, 1); /* e_phnum */
    ByteBufWriteU16LE(&out, 0); /* e_shentsize */
    ByteBufWriteU16LE(&out, 0); /* e_shnum */
    ByteBufWriteU16LE(&out, 0); /* e_shstrndx */

    /* ================= Elf64_Phdr ================= */

    ByteBufWriteU32LE(&out, PT_LOAD);
    ByteBufWriteU32LE(&out, PF_X | PF_R);
    ByteBufWriteU64LE(&out, 0);         /* p_offset */
    ByteBufWriteU64LE(&out, BASE_ADDR); /* p_vaddr */
    ByteBufWriteU64LE(&out, BASE_ADDR); /* p_paddr */
    ByteBufWriteU64LE(&out, totalSize); /* p_filesz */
    ByteBufWriteU64LE(&out, totalSize); /* p_memsz */
    ByteBufWriteU64LE(&out, 0x1000);    /* p_align */

    /* ================= code ================= */

    ByteBufWriteBytes(&out, result->code.data, result->code.len);

    return out;
}

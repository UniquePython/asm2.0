#include "bytebuf.h"

#include <stdlib.h>

#include "mem.h"

#define BYTEBUF_INITIAL_CAP 32

void ByteBufInit(ByteBuf *buf)
{
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

void ByteBufFree(ByteBuf *buf)
{
    free(buf->data);
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

void ByteBufWriteU8(ByteBuf *buf, u8 value)
{
    if (buf->len == buf->cap)
    {
        usize newCap = buf->cap == 0 ? BYTEBUF_INITIAL_CAP : buf->cap * 2;
        buf->data = Realloc(buf->data, newCap);
        buf->cap = newCap;
    }

    buf->data[buf->len] = value;
    buf->len++;
}

void ByteBufWriteU16LE(ByteBuf *buf, u16 value)
{
    ByteBufWriteU8(buf, (u8)(value & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 8) & 0xFF));
}

void ByteBufWriteU32LE(ByteBuf *buf, u32 value)
{
    ByteBufWriteU8(buf, (u8)(value & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 8) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 16) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 24) & 0xFF));
}

void ByteBufWriteU64LE(ByteBuf *buf, u64 value)
{
    ByteBufWriteU8(buf, (u8)(value & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 8) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 16) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 24) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 32) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 40) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 48) & 0xFF));
    ByteBufWriteU8(buf, (u8)((value >> 56) & 0xFF));
}

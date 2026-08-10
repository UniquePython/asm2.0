#ifndef BYTEBUF_H_
#define BYTEBUF_H_

#include "types.h"

typedef struct byte_buf_t
{
    u8 *data;
    usize len;
    usize cap;
} ByteBuf;

void ByteBufInit(ByteBuf *buf);
void ByteBufFree(ByteBuf *buf);

void ByteBufWriteU8(ByteBuf *buf, u8 value);
void ByteBufWriteU16LE(ByteBuf *buf, u16 value);
void ByteBufWriteU32LE(ByteBuf *buf, u32 value);
void ByteBufWriteU64LE(ByteBuf *buf, u64 value);

#endif /* BYTEBUF_H_ */

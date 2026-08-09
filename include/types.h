#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned char uchar;

typedef size_t usize;

#define I8_FMT "%" PRIi8
#define I8_FMT_LN I8_FMT "\n"

#define I16_FMT "%" PRIi16
#define I16_FMT_LN I16_FMT "\n"

#define I32_FMT "%" PRIi32
#define I32_FMT_LN I32_FMT "\n"

#define I64_FMT "%" PRIi64
#define I64_FMT_LN I64_FMT "\n"

#define U8_FMT "%" PRIu8
#define U8_FMT_LN U8_FMT "\n"

#define U16_FMT "%" PRIu16
#define U16_FMT_LN U16_FMT "\n"

#define U32_FMT "%" PRIu32
#define U32_FMT_LN U32_FMT "\n"

#define U64_FMT "%" PRIu64
#define U64_FMT_LN U64_FMT "\n"

#define UCHAR_FMT "%hhu"
#define UCHAR_FMT_LN UCHAR_FMT "\n"

#define BOOL_FMT "%s"
#define BOOL_FMT_LN BOOL_FMT "\n"
#define BOOL_ARG(boolean) ((boolean) ? "true" : "false")

#endif /* TYPES_H_ */

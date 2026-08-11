#ifndef ELFWRITER_H_
#define ELFWRITER_H_

#include "types.h"
#include "bytebuf.h"
#include "codegen.h"

ByteBuf BuildElf(const CodegenResult *result);

#endif /* ELFWRITER_H_ */

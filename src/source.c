#include "source.h"

#include "mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool SourceLoad(const char *filepath, Source *out)
{
    FILE *file = fopen(filepath, "rb");
    if (!file)
        return false;

    if (fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return false;
    }

    long fileLen = ftell(file);
    if (fileLen < 0)
    {
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0)
    {
        fclose(file);
        return false;
    }

    char *data = Alloc((u32)fileLen + 1);

    u32 len = (u32)fileLen;

    if (fread(data, 1, len, file) != len)
    {
        free(data);
        fclose(file);
        return false;
    }

    data[len] = '\0';

    fclose(file);

    usize filepathLen = strlen(filepath);

    char *filepathCopy = Alloc(filepathLen + 1);

    memcpy(filepathCopy, filepath, filepathLen + 1);

    *out = (Source){
        .filepath = filepathCopy,
        .data = data,
        .len = len,
    };

    return true;
}

void SourceFree(Source *source)
{
    free(source->filepath);
    free(source->data);

    *source = (Source){0};
}

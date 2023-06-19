#include "bitarray.h"
#include <nds/system.h>
#include <stdlib.h>
#include <stdio.h>

BitArray bitArrayCreate(size_t bitCount)
{
    BitArray array;
    array.array = (uint8_t *)calloc(bitCount, 1);
    array.bitCount = bitCount;
    return array;
}

void bitArrayDelete(BitArray *array)
{
    free(array->array);
}

void bitArraySet(BitArray *array, size_t index)
{
    array->array[index / 8] |= (1 << (index % 8));
}

void bitArrayClear(BitArray *array, size_t index)
{
    array->array[index / 8] &= ~(1 << (index % 8));
}

bool bitArrayGet(BitArray *array, size_t index)
{
    if (array->array[index / 8] & (1 << index % 8))
        return true;
    return false;
}

void bitArrayWrite(BitArray array, const char *file)
{
    FILE *fp = fopen(file, "w");
    if (!fp)
    {
        FILE *fpError = fopen("error.log", "w");
        if (fpError)
        {
            // write error into an error file
            fprintf(fpError, "Failed to open file %s for writing bit array", file);
            fclose(fpError);
        }
        systemShutDown();
    }

    for (size_t i = 0; i < array.bitCount; ++i)
        fputc((bitArrayGet(&array, i)) ? '1' : '0', fp);
    fclose(fp);
}

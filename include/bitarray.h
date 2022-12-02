#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    size_t bitCount;
    uint8_t *array;
} BitArray;

// bit count must be divisible by 8 for this to work
BitArray bitArrayCreate(size_t bitCount);
void bitArrayDelete(BitArray *array);

void bitArraySet(BitArray *array, size_t index);
void bitArrayClear(BitArray *array, size_t index);
bool bitArrayGet(BitArray *array, size_t index);
void bitArrayWrite(BitArray array, const char *file);

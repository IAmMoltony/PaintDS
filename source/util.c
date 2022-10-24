#include "util.h"

void swapInt(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
}
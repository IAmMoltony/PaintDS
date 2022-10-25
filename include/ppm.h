#pragma once

#include "graphics.h"
#include <stdio.h>

typedef struct
{
    u8 r, g, b;
} PPMPixel;

typedef struct
{
    int w, h;
    PPMPixel *pixels;
} PPMImage;

#define RGB_COMPONENT_COLOR 255

PPMImage *ppmLoad(const char *filename);
void ppmDraw(FrameBuffer fb, PPMImage *img, int x, int y);
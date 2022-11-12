#pragma once
#include <nds.h>
#include <stdlib.h>
#include <math.h>

typedef u16 *FrameBuffer;

void gfxPutPixel(FrameBuffer fb, int x, int y, u16 color);
void gfxClear(FrameBuffer fb, u16 color);
void gfxFillRect(FrameBuffer fb, int x, int y, int w, int h, u16 color);
void gfxStrokeRect(FrameBuffer fb, int x, int y, int w, int h, u16 color);
void gfxDrawLine(FrameBuffer fb, int x1, int y1, int x2, int y2, u16 color);
void gfxDrawLineThickness(FrameBuffer fb, int x1, int y1, int x2, int y2, u16 color, u8 thickness);
void gfxFloodFill(FrameBuffer fb, int x, int y, u16 color, u16 colorToFill);

u16 gfxGetPixel(FrameBuffer fb, int x, int y);
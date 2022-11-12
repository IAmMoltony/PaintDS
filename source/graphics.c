#include "graphics.h"

void gfxPutPixel(FrameBuffer fb, int x, int y, u16 color)
{
    if (x < 0 || y < 0 || x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
        return; // not draw outside of screen

    fb[x + y * 256] = color;
}

void gfxFillRect(FrameBuffer fb, int x, int y, int w, int h, u16 color)
{
    for (int i = 0; i < w; ++i)
        for (int j = 0; j < h; ++j)
            gfxPutPixel(fb, x + i, y + j, color);
}

void gfxStrokeRect(FrameBuffer fb, int x, int y, int w, int h, u16 color)
{
    for (int i = 0; i < w; ++i)
        for (int j = 0; j < h; ++j)
            if (i == 0 || j == 0 || i == w - 1 || j == h - 1)
                gfxPutPixel(fb, x + i, y + j, color);
}

void gfxDrawLine(FrameBuffer fb, int x1, int y1, int x2, int y2, u16 color)
{
    gfxDrawLineThickness(fb, x1, y1, x2, y2, color, 1);
}

static void drawThickPixel(FrameBuffer fb, int x, int y, u16 color, u8 thickness)
{
    int ax = x - thickness / 2;
    int ay = y - thickness / 2;
    gfxFillRect(fb, ax, ay, thickness, thickness, color);
}

void gfxDrawLineThickness(FrameBuffer fb, int x1, int y1, int x2, int y2, u16 color, u8 thickness)
{
    // https://ghost-together.medium.com/how-to-code-your-first-algorithm-draw-a-line-ca121f9a1395
    // i spent many hours trying to find this algorithm

    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;

    // line deltas
    dx = x2 - x1;
    dy = y2 - y1;

    // positive copy of deltas
    dx1 = abs(dx);
    dy1 = abs(dy);

    // error intervals for both axis
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    // x axis dominant
    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }

        drawThickPixel(fb, x, y, color, thickness); // first pixel

        // rasterize line
        for (i = 0; x < xe; ++i)
        {
            ++x;

            if (px < 0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                    ++y;
                else
                    --y;

                px = px + 2 * (dy1 - dx1);
            }

            drawThickPixel(fb, x, y, color, thickness);
        }
    }
    // y axis dominant
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }

        drawThickPixel(fb, x, y, color, thickness); // first pixel

        // rasterize line
        for (i = 0; y < ye; ++i)
        {
            ++y;

            if (py <= 0)
                py = py + 2 * dx1;
            else
            {
                if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
                    ++x;
                else
                    --x;

                py = py + 2 * (dx1 - dy1);
            }

            drawThickPixel(fb, x, y, color, thickness);
        }
    }
}

void gfxFloodFill(FrameBuffer fb, int x, int y, u16 color, u16 colorToFill)
{
    if (colorToFill == color)
        return;

    if (x < 0 || y < 0 || x > 256 || y > 192)
        return;

    if (gfxGetPixel(fb, x, y) == colorToFill)
    {
        gfxPutPixel(fb, x, y, color);
        gfxFloodFill(fb, x + 1, y, color, colorToFill);
        gfxFloodFill(fb, x - 1, y, color, colorToFill);
        gfxFloodFill(fb, x, y + 1, color, colorToFill);
        gfxFloodFill(fb, x, y - 1, color, colorToFill);
    }
}

u16 gfxGetPixel(FrameBuffer fb, int x, int y)
{
    return fb[x + y * 256];
}
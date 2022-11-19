#include "graphics.h"

void gfxPutPixel(FrameBuffer fb, int x, int y, u16 color)
{
    if (x < 0 || y < 0 || x > SCREEN_WIDTH || y > SCREEN_HEIGHT)
        return; // not draw outside of screen

    fb[x + y * 256] = color;
}

void gfxClear(FrameBuffer fb, u16 color)
{
    for (int i = 0; i < 256 * 192; ++i)
        fb[i] = color;
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

void gfxDrawTriangle(FrameBuffer fb, int x1, int y1, int x2, int y2, int x3, int y3, u16 color)
{
    gfxDrawLine(fb, x1, y1, x2, y2, color);
    gfxDrawLine(fb, x2, y2, x3, y3, color);
    gfxDrawLine(fb, x3, y3, x1, y1, color);
}

typedef struct
{
    int x1, y1, x2, y2;
} TriangleEdge;

typedef struct
{
    int x1, x2;
} TriangleSpan;

static TriangleEdge createTriangleEdge(int x1, int y1, int x2, int y2)
{
    TriangleEdge e;

    if (y1 < y2)
    {
        e.x1 = x1;
        e.y1 = y1;
        e.x2 = x2;
        e.y2 = y2;
    }
    else
    {
        e.x1 = x2;
        e.y1 = y2;
        e.x2 = x1;
        e.y2 = y1;
    }

    return e;
}

static TriangleSpan createTriangleSpan(int x1, int x2)
{
    TriangleSpan s;

    if (x1 < x2)
    {
        s.x1 = x1;
        s.x2 = x2;
    }
    else
    {
        s.x1 = x2;
        s.x2 = x1;
    }

    return s;
}

void drawSpan(FrameBuffer fb, u16 color, TriangleSpan span, int y)
{
    int xDiff = span.x2 - span.x1;
    if (!xDiff)
        return;

    for (int x = span.x1; x < span.x2; ++x)
        gfxPutPixel(fb, x, y, color);
}

void drawSpansBetweenEdges(FrameBuffer fb, u16 color, TriangleEdge e1, TriangleEdge e2)
{
    float e1ydiff = (float)(e1.y2 - e1.y1);
    if (!e1ydiff)
        return;

    float e2ydiff = (float)(e2.y2 - e2.y1);
    if (!e2ydiff)
        return;

    float e1xdiff = (float)(e1.x2 - e1.x1);
    float e2xdiff = (float)(e2.x2 - e2.x1);

    float fact1 = (float)(e2.y1 - e1.y1) / e1ydiff;
    float factStep1 = 1.0f / e1ydiff;
    float fact2 = 0.0f;
    float factStep2 = 1.0f / e2ydiff;

    for (int y = e2.y1; y < e2.y2; ++y)
    {
        TriangleSpan span = createTriangleSpan(e1.x1 + (int)(e1xdiff * fact1), e2.x1 + (int)(e2xdiff * fact2));
        drawSpan(fb, color, span, y);

        fact1 += factStep1;
        fact2 += factStep2;
    }
}

void gfxFillTriangle(FrameBuffer fb, int x1, int y1, int x2, int y2, int x3, int y3, u16 color)
{
    // i spent like an hour tryna find this implementation
    // original https://joshbeam.com/articles/triangle_rasterization/
    // rewritten a bit to work in c

    TriangleEdge edges[3] = {
        createTriangleEdge(x1, y1, x2, y2),
        createTriangleEdge(x2, y2, x3, y3),
        createTriangleEdge(x3, y3, x1, y1),

    };

    int maxLen = 0;
    int longEdge = 0;
    for (int i = 0; i < 3; ++i)
    {
        int len = edges[i].y2 - edges[i].y1;
        if (len > maxLen)
        {
            maxLen = len;
            longEdge = i;
        }
    }

    int shortEdge1 = (longEdge + 1) % 3;
    int shortEdge2 = (longEdge + 2) % 3;

    drawSpansBetweenEdges(fb, color, edges[longEdge], edges[shortEdge1]);
    drawSpansBetweenEdges(fb, color, edges[longEdge], edges[shortEdge2]);
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
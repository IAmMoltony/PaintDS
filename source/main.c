#include <nds.h>
#include <fat.h>
#include <filesystem.h>
#include <maxmod9.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "soundbank.h"
#include "colors.h"
#include "ppm.h"
#include "background.h"
#define COLORS_PER_PAGE 7
#define MAX_COLOR_PAGE 1

typedef enum
{
    toolPencil = 0,
    toolEraser = 1,
    toolFill = 2,
    toolShapes = 3,
} Tool;

FrameBuffer fb;
FrameBuffer picture;

u16 colors[] = {
    BLACK, RED, GREEN, BLUE, CYAN, MAGENTA, YELLOW,
    GRAY, DARKRED, DARKGREEN, DARKBLUE, DARKCYAN, DARKMAGENTA, DARKYELLOW};

static PPMImage *imgArrow;

u8 colorPage = 0;

void hudDrawColors(u8 selected)
{
    for (u8 i = 0; i < COLORS_PER_PAGE; ++i)
    {
        u16 color = colors[colorPage * COLORS_PER_PAGE + i];

        gfxFillRect(fb, 10 + 15 * i, 2, 12, 12, color);
        if (selected == colorPage * COLORS_PER_PAGE + i)
            gfxStrokeRect(fb, 10 + 15 * i, 2, 12, 12, GREEN);
        else
            gfxStrokeRect(fb, 10 + 15 * i, 2, 12, 12, BLACK);
    }

    ppmDraw(fb, imgArrow, 3, 3);
    ppmDrawMirrorX(fb, imgArrow, 114, 3);
}

void hudColorSelect(touchPosition pos, u8 *selectedColor)
{
    u16 x = pos.px;
    u16 y = pos.py;
    if (!(y <= 14 || y >= 2))
        return;

    // scroll back button
    if (x >= 3 && y >= 3 && x <= 3 + imgArrow->w && y <= 3 + imgArrow->h)
    {
        if (colorPage - 1 < 0)
            colorPage = MAX_COLOR_PAGE;
        else
            --colorPage;
        *selectedColor = colorPage * COLORS_PER_PAGE;
        return;
    }
    // scroll forward button
    else if (x >= 114 && y >= 3 && x <= 114 + imgArrow->w && y <= 3 + imgArrow->h)
    {
        if (colorPage + 1 > MAX_COLOR_PAGE)
            colorPage = 0;
        else
            ++colorPage;
        *selectedColor = colorPage * COLORS_PER_PAGE;
        return;
    }

    for (u8 i = 0; i < COLORS_PER_PAGE; ++i)
    {
        if (x >= 10 + 15 * i && x <= 10 + 15 * i + 12)
        {
            *selectedColor = colorPage * COLORS_PER_PAGE + i;
            return;
        }
    }
}

void hudLineThicknessSelect(touchPosition pos, u8 *lineThickness)
{
    u16 x = pos.px;
    u16 y = pos.py;
    if (!(y <= 14 || y >= 2))
        return;

    for (u8 i = 0; i < 3; ++i)
    {
        if (x >= 256 - 44 + 15 * i && x <= 256 - 44 + 15 * i + 12)
        {
            *lineThickness = i;
            return;
        }
    }
}

void hudChooseTool(touchPosition pos, Tool *tool)
{
    u16 x = pos.px;
    u16 y = pos.py;
    if (!(y >= 192 - 16 || y <= 192))
        return;

    for (Tool i = toolPencil; i <= toolShapes; ++i)
    {
        if (x >= 2 + 15 * i && x <= 2 + 15 * i + 12)
        {
            *tool = i;
            return;
        }
    }
}

void hudShapeSelect(touchPosition pos, u8 *shape)
{
    u16 x = pos.px;
    u16 y = pos.py;
    if (!(y >= 2 && y <= 14))
        return;

    for (u8 i = 0; i < 5; ++i)
    {
        u8 btnPos = 242 - i * 14;
        if (x >= btnPos && x <= btnPos + 12)
        {
            *shape = i;
            return;
        }
    }
}

int main(int argc, char **argv)
{
    if (!nitroFSInit(NULL))
    {
        consoleDemoInit();
        printf("nitrofs init failed");

        while (true)
            ;
    }

    if (!fatInitDefault())
    {
        consoleDemoInit();
        printf("Oh no!\n\n");
        printf("Failed to initialize FAT.\n");
        printf("Make sure your ROM is properly patched\n");
        printf("and the SD card is inserted.");

        while (true)
            ;
    }

    // go into paintds_data directory
    struct stat pds_data_st = {0};
    if (stat("paintds_data", &pds_data_st) == -1)
        mkdir("paintds_data", 0700);
    chdir("paintds_data");

    mmInitDefault("nitro:/soundbank.bin");

    mmLoadEffect(SFX_PENCIL);
    mm_sound_effect sndPencil = {
        {SFX_PENCIL},
        (int)(1.0f * (1 << 10)),
        0,
        255,
        128};

    videoSetMode(MODE_5_2D);
    videoSetModeSub(MODE_5_2D);

    vramSetBankA(VRAM_A_MAIN_BG);
    vramSetBankC(VRAM_C_SUB_BG);

    int bgTop = bgInit(2, BgType_Bmp8, BgSize_B8_512x256, 0, 0);
    dmaCopy(backgroundBitmap, bgGetGfxPtr(bgTop), 512 * 192);
    dmaCopy(backgroundPal, BG_PALETTE, 256 * 2);

    int bgSub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    FrameBuffer vramSub = bgGetGfxPtr(bgSub);

    // allocate framebuffer and pictue buffer
    fb = malloc(256 * 192 * sizeof(u16));
    picture = malloc(256 * 192 * sizeof(u16));

    gfxClear(fb, WHITE);
    gfxClear(picture, WHITE);

    // load some assets
    PPMImage *imgPencil = ppmLoad("nitro:/graphics/pencil.ppm");
    PPMImage *imgEraser = ppmLoad("nitro:/graphics/eraser.ppm");
    PPMImage *imgEraseAll = ppmLoad("nitro:/graphics/eraseall.ppm");
    PPMImage *imgFill = ppmLoad("nitro:/graphics/fill.ppm");
    PPMImage *imgShapes = ppmLoad("nitro:/graphics/shapes.ppm");
    imgArrow = ppmLoad("nitro:/graphics/arrow.ppm");
    PPMImage *imgFilledSquare = ppmLoad("nitro:/graphics/filledsquare.ppm");
    PPMImage *imgNotFilledSquare = ppmLoad("nitro:/graphics/notfilledsquare.ppm");
    PPMImage *imgLine = ppmLoad("nitro:/graphics/line.ppm");
    PPMImage *imgNotfIlledTriangle = ppmLoad("nitro:/graphics/notfilledtriangle.ppm");
    PPMImage *imgFilledTriangle = ppmLoad("nitro:/graphics/filledtriangle.ppm");

    int oldTouchX = -1;
    int oldTouchY = -1;
    int shapesStartX = -1;
    int shapesStartY = -1;
    touchPosition shapesTouchPos = {0, 0, 0, 0, 0, 0};
    u16 bgScrollX = 0;
    bool showHud = true;
    bool eraserFill = false;
    u8 selectedColor = 0;
    u8 shapesSelectedShape = 0; // 0 = filled square
                                // 1 = not filled square
                                // 2 = line
                                // 3 = filled triangle
                                // 4 = not filled triangle
    u8 lineThickness = 0;
    u16 frames = 0;
    Tool tool = toolPencil;
    while (true)
    {
        ++frames;

        scanKeys();
        u32 kdown = keysDown();

        // toggle showing HUD when pressing L or R
        if (kdown & KEY_L || kdown & KEY_R)
            showHud = !showHud;
        else if (kdown & KEY_TOUCH)
        {
            touchPosition pos;
            touchRead(&pos);

            if (pos.py < 16 && showHud)
            {
                switch (tool)
                {
                case toolPencil:
                case toolFill:
                case toolShapes:
                    hudColorSelect(pos, &selectedColor);
                    break;
                case toolEraser:
                    // erase all button
                    if (pos.py >= 2 && pos.px >= 2 && pos.py <= 14 && pos.px <= 14)
                        gfxClear(picture, WHITE);
                    // eraser fill
                    else if (pos.py >= 2 && pos.px >= 16 && pos.py <= 14 && pos.px <= 30)
                        eraserFill = !eraserFill;
                    break;
                }

                switch (tool)
                {
                case toolPencil:
                case toolEraser:
                    hudLineThicknessSelect(pos, &lineThickness);
                    break;
                case toolShapes:
                    hudShapeSelect(pos, &shapesSelectedShape);
                    break;
                }
            }
            else if (pos.py > 192 - 16 && showHud)
                hudChooseTool(pos, &tool);
            else if ((pos.py < 192 - 16 && pos.py > 16 && showHud) || !showHud)
            {
                switch (tool)
                {
                case toolFill:
                    gfxFloodFill(picture, pos.px, pos.py, colors[selectedColor], gfxGetPixel(picture, pos.px, pos.py));

                    ppmUnload(imgArrow);
                    ppmLoad("nitro:/graphics/arrow.ppm");
                    break;
                case toolEraser:
                    if (eraserFill)
                        gfxFloodFill(picture, pos.px, pos.py, WHITE, gfxGetPixel(picture, pos.px, pos.py));
                    break;
                }
            }
        }

        // scroll background on top screen
        bgSetScroll(bgTop, bgScrollX++, 0);
        if (bgScrollX > 255)
            bgScrollX = 0;
        bgUpdate();

        // draw stuff if touching the screen
        if (keysHeld() & KEY_TOUCH)
        {
            touchPosition pos;
            touchRead(&pos);

            if (!((pos.py < 16 || pos.py > 192 - 16) && showHud))
            {
                int x1 = (oldTouchX == -1) ? pos.px : oldTouchX;
                int y1 = (oldTouchY == -1) ? pos.py : oldTouchY;

                u8 thickness;
                switch (lineThickness)
                {
                default:
                    thickness = 1;
                    lineThickness = 0;
                    break;
                case 1:
                    thickness = 4;
                    break;
                case 2:
                    thickness = 8;
                    break;
                }

                switch (tool)
                {
                case toolPencil:
                    gfxDrawLineThickness(picture, x1, y1, pos.px, pos.py, colors[selectedColor], thickness);
                    break;
                case toolEraser:
                    gfxDrawLineThickness(picture, x1, y1, pos.px, pos.py, WHITE, thickness);
                    break;
                case toolShapes:
                    touchRead(&shapesTouchPos);
                    if (shapesStartX == -1 || shapesStartY == -1)
                    {
                        shapesStartX = pos.px;
                        shapesStartY = pos.py;
                    }
                    break;
                }
            }

            if (!(pos.px == oldTouchX && pos.py == oldTouchY) &&    // if we arent just holding in one position
                !((pos.py < 16 || pos.py > 192 - 16) && showHud) && // and if we are showing HUD, ignore touching it
                tool != toolShapes && tool != toolFill)             // and these tools cant produce pencil sfx
                mmEffectEx(&sndPencil);

            oldTouchX = pos.px;
            oldTouchY = pos.py;
        }
        else
            oldTouchX = oldTouchY = -1;

        if (keysUp() & KEY_TOUCH)
        {
            switch (tool)
            {
            case toolShapes:
                if (shapesStartX != -1 && shapesStartY != -1)
                {
                    touchPosition pos = shapesTouchPos;

                    switch (shapesSelectedShape)
                    {
                    case 0: // filled rect
                    {
                        int startX = shapesStartX;
                        int startY = shapesStartY;
                        int endX = pos.px;
                        int endY = pos.py;
                        if (startX > endX)
                        {
                            int temp = startX;
                            startX = endX;
                            endX = temp;
                        }
                        if (startY > endY)
                        {
                            int temp = startY;
                            startY = endY;
                            endY = temp;
                        }
                        for (int i = startX; i <= endX; ++i)
                            for (int j = startY; j <= endY; ++j)
                                gfxPutPixel(picture, i, j, colors[selectedColor]);
                        break;
                    }
                    case 1: // not filled rect
                    {
                        int startX = shapesStartX;
                        int startY = shapesStartY;
                        int endX = pos.px;
                        int endY = pos.py;
                        if (startX > endX)
                        {
                            int temp = startX;
                            startX = endX;
                            endX = temp;
                        }
                        if (startY > endY)
                        {
                            int temp = startY;
                            startY = endY;
                            endY = temp;
                        }
                        for (int i = startX; i <= endX; ++i)
                            for (int j = startY; j <= endY; ++j)
                                if (i == shapesStartX || i == pos.px || j == shapesStartY || j == pos.py)
                                    gfxPutPixel(picture, i, j, colors[selectedColor]);
                        break;
                    }
                    case 2: // line
                        gfxDrawLine(picture, shapesStartX, shapesStartY, pos.px, pos.py, colors[selectedColor]);
                        break;
                    case 3: // filled triangle
                        gfxFillTriangle(picture, shapesStartX + (pos.px - shapesStartX) / 2, shapesStartY, shapesStartX, pos.py, pos.px, pos.py, colors[selectedColor]);
                        break;
                    case 4: // not filled triangle
                        gfxDrawTriangle(picture, shapesStartX + (pos.px - shapesStartX) / 2, shapesStartY, shapesStartX, pos.py, pos.px, pos.py, colors[selectedColor]);
                        break;
                    }
                    shapesStartX = -1;
                    shapesStartY = -1;
                }
                break;
            }
        }

        // copy picture buffer into frame buffer
        for (int x = 0; x < 256; ++x)
            for (int y = 0; y < 192; ++y)
                fb[x + y * 256] = picture[x + y * 256];

        // HUD rendering
        if (showHud)
        {
            gfxFillRect(fb, 0, 192 - 16, 256, 16, ARGB16(1, 31, 31, 20));
            gfxDrawLine(fb, 0, 192 - 16, 256, 192 - 16, ARGB16(1, 0, 0, 0));

            gfxFillRect(fb, 0, 0, 256, 16, ARGB16(1, 20, 29, 31));
            gfxDrawLine(fb, 0, 16, 256, 16, ARGB16(1, 0, 0, 0));

            switch (tool)
            {
            case toolPencil:
            case toolFill:
            case toolShapes:
                // draw colors
                hudDrawColors(selectedColor);
                break;
            case toolEraser:
                // draw erase all button
                ppmDraw(fb, imgEraseAll, 2, 2);

                // draw fill eraser button
                ppmDraw(fb, imgFill, 16, 2);
                if (eraserFill)
                    gfxStrokeRect(fb, 16, 2, 12, 12, GREEN);
                break;
            }

            switch (tool)
            {
            case toolPencil:
            case toolEraser:
                // draw line thicknesses
                gfxStrokeRect(fb, 256 - 44, 2, 12, 12, lineThickness == 0 ? GREEN : BLACK);
                gfxPutPixel(fb, 256 - 39, 7, colors[selectedColor]);

                gfxStrokeRect(fb, 256 - 29, 2, 12, 12, lineThickness == 1 ? GREEN : BLACK);
                gfxFillRect(fb, 256 - 25, 5, 4, 4, colors[selectedColor]);

                gfxStrokeRect(fb, 256 - 14, 2, 12, 12, lineThickness == 2 ? GREEN : BLACK);
                gfxFillRect(fb, 256 - 12, 4, 8, 8, colors[selectedColor]);
                break;
            case toolShapes:
                // draw shape images
                ppmDraw(fb, imgFilledSquare, 242, 2);
                gfxStrokeRect(fb, 242, 2, 12, 12, (shapesSelectedShape == 0) ? GREEN : BLACK);

                ppmDraw(fb, imgNotFilledSquare, 228, 2);
                gfxStrokeRect(fb, 228, 2, 12, 12, (shapesSelectedShape == 1) ? GREEN : BLACK);

                ppmDraw(fb, imgLine, 214, 2);
                gfxStrokeRect(fb, 214, 2, 12, 12, (shapesSelectedShape == 2) ? GREEN : BLACK);

                ppmDraw(fb, imgFilledTriangle, 200, 2);
                gfxStrokeRect(fb, 200, 2, 12, 12, (shapesSelectedShape == 3) ? GREEN : BLACK);

                ppmDraw(fb, imgNotfIlledTriangle, 186, 2);
                gfxStrokeRect(fb, 186, 2, 12, 12, (shapesSelectedShape == 4) ? GREEN : BLACK);
                break;
            }

            // draw tool images
            ppmDraw(fb, imgPencil, 2, 192 - 14 - ((tool == toolPencil) ? 3 : 0));
            ppmDraw(fb, imgEraser, 16, 192 - 14 - ((tool == toolEraser) ? 3 : 0));
            ppmDraw(fb, imgFill, 30, 192 - 14 - ((tool == toolFill) ? 3 : 0));
            ppmDraw(fb, imgShapes, 44, 192 - 14 - ((tool == toolShapes) ? 3 : 0));
        }

        if (tool == toolShapes && shapesStartX != -1 && shapesStartY != -1)
        {
            touchPosition pos = shapesTouchPos;

            switch (shapesSelectedShape)
            {
            case 0: // filled rect
            {
                int startX = shapesStartX;
                int startY = shapesStartY;
                int endX = pos.px;
                int endY = pos.py;
                if (startX > endX)
                {
                    int temp = startX;
                    startX = endX;
                    endX = temp;
                }
                if (startY > endY)
                {
                    int temp = startY;
                    startY = endY;
                    endY = temp;
                }
                for (int i = startX; i <= endX; ++i)
                    for (int j = startY; j <= endY; ++j)
                        gfxPutPixel(fb, i, j, colors[selectedColor]);
                break;
            }
            case 1: // not filled rect
            {
                int startX = shapesStartX;
                int startY = shapesStartY;
                int endX = pos.px;
                int endY = pos.py;
                if (startX > endX)
                {
                    int temp = startX;
                    startX = endX;
                    endX = temp;
                }
                if (startY > endY)
                {
                    int temp = startY;
                    startY = endY;
                    endY = temp;
                }
                for (int i = startX; i <= endX; ++i)
                    for (int j = startY; j <= endY; ++j)
                        if (i == shapesStartX || i == pos.px || j == shapesStartY || j == pos.py)
                            gfxPutPixel(fb, i, j, colors[selectedColor]);
                break;
            }
            case 2: // line
                gfxDrawLine(fb, shapesStartX, shapesStartY, pos.px, pos.py, colors[selectedColor]);
                break;
            case 3: // filled triangle
                gfxFillTriangle(fb, shapesStartX + (pos.px - shapesStartX) / 2, shapesStartY, shapesStartX, pos.py, pos.px, pos.py, colors[selectedColor]);
                break;
            case 4: // not filled triangle
                gfxDrawTriangle(fb, shapesStartX + (pos.px - shapesStartX) / 2, shapesStartY, shapesStartX, pos.py, pos.px, pos.py, colors[selectedColor]);
                break;
            }
        }

        // copy frame buffer into bottom screen
        for (int x = 0; x < 256; ++x)
            for (int y = 0; y < 192; ++y)
                vramSub[x + y * 256] = fb[x + y * 256];

        swiWaitForVBlank();
    }
    return 0;
}

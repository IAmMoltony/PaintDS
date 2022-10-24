#include <nds.h>
#include <fat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include "colors.h"
#include "graphics.h"
#include "background.h"

FrameBuffer fb;
FrameBuffer picture;

int main(int argc, char **argv)
{
    if (!fatInitDefault())
    {
        consoleDemoInit();
        printf("oh no!\n\n");
        printf("failed to initialize FAT.\n");
        printf("make sure your rom is properly patched\n");
        printf("and the SD card is inserted.");

        while (true)
            ;
    }

    // go into paintds_data directory
    struct stat pds_data_st = {0};
    if (stat("paintds_data", &pds_data_st) == -1)
        mkdir("paintds_data", 0700);
    chdir("paintds_data");

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

    // clear buffers
    for (int i = 0; i < 256 * 192; ++i)
    {
        fb[i] = WHITE;
        picture[i] = WHITE;
    }

    int oldTouchX = -1;
    int oldTouchY = -1;
    u16 bgScrollX = 0;
    bool showHud = true;
    while (true)
    {
        scanKeys();
        u32 kdown = keysDown();

        // toggle showing HUD when pressing L or R
        if (kdown & KEY_L || kdown & KEY_R)
            showHud = !showHud;

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

            if (oldTouchX == -1 || oldTouchY == -1)
                gfxPutPixel(picture, pos.px, pos.py, BLACK);
            else
                gfxDrawLine(picture, oldTouchX, oldTouchY, pos.px, pos.py, BLACK);

            oldTouchX = pos.px;
            oldTouchY = pos.py;
        }
        else
            oldTouchX = oldTouchY = -1;

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
        }

        // copy frame buffer into bottom screen
        for (int x = 0; x < 256; ++x)
            for (int y = 0; y < 192; ++y)
                vramSub[x + y * 256] = fb[x + y * 256];

        swiWaitForVBlank();
    }
    return 0;
}
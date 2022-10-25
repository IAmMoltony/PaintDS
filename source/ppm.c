#include "ppm.h"

PPMImage *ppmLoad(const char *filename)
{
    // taken from stackoverflow

    char buff[16];
    PPMImage *img;
    FILE *fp;
    int c, rgb_comp_color;
    // open PPM file for reading
    fp = fopen(filename, "rb");
    if (!fp)
    {
        while (true)
            ;
    }

    // read image format
    if (!fgets(buff, sizeof(buff), fp))
    {
        perror(filename);
        while (true)
            ;
    }

    // check the image format
    if (buff[0] != 'P' || buff[1] != '6')
    {
        while (true)
            ;
    }

    // alloc memory for image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img)
    {
        while (true)
            ;
    }

    // check for comments
    c = getc(fp);
    while (c == '#')
    {
        while (getc(fp) != '\n')
            ;
        c = getc(fp);
    }

    ungetc(c, fp);
    // read image size information
    if (fscanf(fp, "%d %d", &img->w, &img->h) != 2)
    {
        while (true)
            ;
    }

    // read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1)
    {
        while (true)
            ;
    }

    // check rgb component depth
    if (rgb_comp_color != RGB_COMPONENT_COLOR)
    {
        while (true)
            ;
    }

    while (fgetc(fp) != '\n')
        ;
    // memory allocation for pixel data
    img->pixels = (PPMPixel *)malloc(img->w * img->h * sizeof(PPMPixel));

    if (!img)
    {
        while (true)
            ;
    }

    // read pixel data from file
    if (fread(img->pixels, 3 * img->w, img->h, fp) != img->h)
    {
        fprintf(stderr, "Error loading image '%s'\n", filename);
        while (true)
            ;
    }

    fclose(fp);
    return img;
}

void ppmDraw(FrameBuffer fb, PPMImage *img, int x, int y)
{
    for (int i = 0; i < img->w; ++i)
        for (int j = 0; j < img->h; ++j)
        {
            PPMPixel pixel = img->pixels[i + j * img->w];
            u8 r = pixel.r;
            u8 g = pixel.g;
            u8 b = pixel.b;
            // magenta = transparent
            if (r == 255 && g == 0 && b == 255)
                continue;
            u16 color = ARGB16(1, r, g, b);
            fb[(i + x) + (j + y) * SCREEN_WIDTH] = color;
        }
}
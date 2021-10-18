/* date = October 17th 2021 11:37 pm */

#ifndef HY3D_IMAGE_H
#define HY3D_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#include "hy3d_base.h"

// NOTE(heyyod): for RGBA
#define IMAGE_TOTAL_SIZE(i) \
i.width * i.height * 4

struct image
{
    u8* pixels;
    i32 width;
    i32 height;
};

function bool LoadImageRGBA(const char *filename, image &imageOut)
{
    u8 *pixels = stbi_load(filename, &imageOut.width, &imageOut.height, 0, STBI_rgb_alpha);
    if (!pixels)
    {
        imageOut = {};
        return false;
    }
    memcpy(imageOut.pixels, pixels, IMAGE_TOTAL_SIZE(imageOut));
    stbi_image_free(pixels);
    return true;
}

#endif //HY3D_IMAGE_H

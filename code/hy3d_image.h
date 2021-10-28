/* date = October 17th 2021 11:37 pm */

#ifndef HY3D_IMAGE_H
#define HY3D_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#include "hy3d_base.h"

// NOTE(heyyod): for RGBA
#define IMAGE_TOTAL_SIZE(i) i.width * i.height * 4 + 4

struct image
{
    u32 width;
    u32 height;
    u32 mipLevels;
    u8* pixels;
};

function_ bool LoadImageRGBA(const char *filename, image *imageOut)
{
    i32 width, height;
    u8 *pixels = stbi_load(filename, &width, &height, 0, STBI_rgb_alpha);
    if (!pixels)
    {
        DebugPrint("ERROR: Could not load image: ");
        DebugPrint(filename);
        return false;
    }
    imageOut->width = (u32)width;
    imageOut->height = (u32)height;
    imageOut->mipLevels = (u32)floor(log2(Max(width, height))) + 1;
    imageOut->pixels = (u8 *)imageOut + sizeof(image);
    memcpy(imageOut->pixels, pixels, IMAGE_TOTAL_SIZE((*imageOut)));
    stbi_image_free(pixels);
    return true;
}

#endif //HY3D_IMAGE_H

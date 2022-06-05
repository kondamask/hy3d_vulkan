/* date = October 17th 2021 11:37 pm */

#ifndef HY3D_IMAGE_H
#define HY3D_IMAGE_H

#define STB_IMAGE_IMPLEMENTATION
#include "extern/stb_image.h"
#include "math.h"
#include "core.h"
#include "asset.h"

// NOTE(heyyod): for RGBA
#define IMAGE_TOTAL_SIZE(i) i.width * i.height * 4 + 4

struct image_header
{
    u32 width;
    u32 height;
    u32 mipLevels;
    u64 offset;
    u8* pixels;
};

static_func bool LoadImageRGBA(const char *filename, asset *asset)
{
    i32 width, height;
    u8 *pixels = stbi_load(filename, &width, &height, 0, STBI_rgb_alpha);
    if (!pixels)
    {
        DebugPrint("ERROR: Could not load image: ");
        DebugPrint(filename);
        return false;
    }

    image_header *image = (image_header *)asset;
    image->width = (u32)width;
    image->height = (u32)height;
    image->mipLevels = (u32)floor(log2(Max(width, height))) + 1;
    image->pixels = (u8 *)image + sizeof(image);
    memcpy(image->pixels, pixels, IMAGE_TOTAL_SIZE((*image)));
    stbi_image_free(pixels);
    return true;
}

#endif //HY3D_IMAGE_H

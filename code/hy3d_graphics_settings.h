/* date = October 23rd 2021 5:04 pm */

#ifndef HY3D_GRAPHICS_SETTINGS_H
#define HY3D_GRAPHICS_SETTINGS_H

enum MSAA_OPTIONS
{
    MSAA_OFF,
    MSAA_2,
    MSAA_4,
    MSAA_8,
    MSAA_16,
    MSAA_32,
    MSAA_64
};

enum CHANGE_GRAPHICS_SETTINGS
{
    CHANGE_NONE = 0,
    CHANGE_MSAA = 1
};

inline CHANGE_GRAPHICS_SETTINGS operator|(CHANGE_GRAPHICS_SETTINGS &a, CHANGE_GRAPHICS_SETTINGS &b)
{
    return (CHANGE_GRAPHICS_SETTINGS)((i32)a | (i32)b);
}

inline CHANGE_GRAPHICS_SETTINGS operator|=(CHANGE_GRAPHICS_SETTINGS &a, CHANGE_GRAPHICS_SETTINGS b)
{
    return (a = a | b);
}

struct graphics_settings
{
    MSAA_OPTIONS msaa;
};

#endif //HY3D_GRAPHICS_SETTINGS_H

#pragma once
#include "hy3d_types.h"

#include <chrono>

#define KILOBYTES(val) (val * 1024LL)
#define MEGABYTES(val) (KILOBYTES(val) * 1024LL)
#define GIGABYTES(val) (MEGABYTES(val) * 1024LL)
#define TERABYTES(val) (GIGABYTES(val) * 1024LL)

struct debug_read_file_result
{
    void *content;
    u32 size;
};

#define DEBUG_READ_FILE(name) debug_read_file_result name(char *filename)
typedef DEBUG_READ_FILE(debug_read_file);

#define DEBUG_WRITE_FILE(name) bool name(char *filename, u32 memorySize, void *memory)
typedef DEBUG_WRITE_FILE(debug_write_file);

#define DEBUG_FREE_FILE(name) void name(void *memory)
typedef DEBUG_FREE_FILE(debug_free_file);

#pragma pack(push, 1)
struct bitmap_header
{
    u16 fileType;
    u32 fileSize;
    u16 reserved1;
    u16 reserved2;
    u32 bitmapOffset;
    u32 size;
    i32 width;
    i32 height;
    u16 planes;
    u16 bitsPerPixel;
    u32 compression;
    u32 sizeOfBitmap;
    i32 horzResolution;
    i32 vertResolution;
    u32 colorsUsed;
    u32 colorsImportant;

    u32 redMask;
    u32 greenMask;
    u32 blueMask;
};
#pragma pack(pop)

struct engine_memory
{
    bool isInitialized;
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;

    debug_read_file *DEBUGReadFile;
    debug_write_file *DEBUGWriteFile;
    debug_free_file *DEBUGFreeFileMemory;
};

struct memory_arena
{
    u8 *base;
    size_t size;
    size_t used;
};

enum KEYBOARD_BUTTON
{
    UP,
    LEFT,
    DOWN,
    RIGHT,
    W,
    A,
    S,
    D,
    Q,
    E,
    R,
    F,
    Z,
    X,
    C,
    V,
    I,
    J,
    K,
    L,
    U,
    O,
    SHIFT,
    CTRL,
    ALT,
    F4,
    ZERO,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    COUNT,
    INVALID
};

struct keyboard
{
    // TODO: USE A SINGLE VARIABLE INSTEAD OF A BOOL ARRAY
    // WE ONLY NEED 1 BIT FOR A KEY
    bool autoRepeatEnabled = false;
    bool isPressed[KEYBOARD_BUTTON::COUNT];

    inline void Clear()
    {
        for (int i = 0; i < KEYBOARD_BUTTON::COUNT; i++)
            isPressed[i] = false;
    }

    inline void ToggleKey(KEYBOARD_BUTTON key)
    {
        isPressed[key] = !isPressed[key];
    }
};

struct mouse
{
    i16 x;
    i16 y;
    bool isInWindow;
    bool leftIsPressed;
    bool rightIsPressed;
    bool wheelUp;

    inline void SetPos(i16 x_, i16 y_)
    {
        x = x_;
        y = y_;
    }

    i32 WheelDelta()
    {
        i32 result = wheelDelta;
        wheelDelta = 0;
        return result;
    }

    void SetWheelDelta(i32 delta)
    {
        wheelDelta = delta;
    }

private:
    i32 wheelDelta;
};

struct engine_input
{
    mouse mouse;
    keyboard keyboard;
};

struct engine_state
{
    memory_arena memoryArena;
};

struct hy3d_engine
{
    engine_input input;
    std::chrono::steady_clock::time_point frameStart;
};

// IMPORTANT:
// This is the way to create to dynamicaly load your code. In short it
// says that there is a function of type X with THESE parameters and someone
// is goind to call it with a function pointer at some point. So we just need
// that function pointer to load our code whenever we want.
// https://hero.handmade.network/episode/code/day021/

// 1. Make a macro that defines a function with the name we pass it
#define UPDATE_AND_RENDER(name) void name(hy3d_engine &e, engine_memory *memory)
// 2. Create a typedef that says: there is a function of type void (the actual type of the function)
//    I want to replace it with <name>. So now the function is of type update_and_render.
//    I can use this to get a pointer to said function.
typedef UPDATE_AND_RENDER(update_and_render);
// This translates to:
// typedef void update_and_render(hy3d_engine &e, engine_memory *memory);
// 3. Create a stub function that prevents the program from crashing it.
UPDATE_AND_RENDER(UpdateAndRenderStub) {}

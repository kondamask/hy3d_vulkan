#pragma once
#include "hy3d_base.h"

#include <chrono>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH_MIN 256
#define WINDOW_HEIGHT_MIN 144

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

#define VULKAN_DRAW_FUNC(name) bool name()
typedef VULKAN_DRAW_FUNC(vulkan_draw_func);

#define VULKAN_UPDATE_FUNC(name) bool name(f32 color[3])
typedef VULKAN_UPDATE_FUNC(vulkan_udate_func);

struct platform_api
{
    vulkan_draw_func *Draw;
    vulkan_udate_func *Update;

#if HY3D_DEBUG
    debug_read_file *DEBUGReadFile;
    debug_write_file *DEBUGWriteFile;
    debug_free_file *DEBUGFreeFileMemory;
#endif
};

struct engine_memory
{
    bool isInitialized;
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;

    platform_api platformAPI;
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
    float color[3];
    float change[3];
};
struct hy3d_engine
{
    engine_input input;
    std::chrono::steady_clock::time_point frameStart;
    bool onResize;
};

#define UPDATE_AND_RENDER(name) void name(hy3d_engine &e, engine_memory *memory)
typedef UPDATE_AND_RENDER(update_and_render);
UPDATE_AND_RENDER(UpdateAndRenderStub) {}
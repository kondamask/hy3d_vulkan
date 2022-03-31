#ifndef HY3D_ENGINE_H
#define HY3D_ENGINE_H 1


#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH_MIN 255
#define WINDOW_HEIGHT_MIN 255

#define MAX_STAGE_BUFFER_SLOTS 100
#define MAX_TRANSFORM_SLOTS 1000

#define VULKAN_UP {0.0f, -1.0f, 0.0f}

#include <chrono>
#include "hy3d_base.h"
#include "hy3d_mesh.h"
#include "hy3d_image.h"
#include "hy3d_math.h"
#include "hy3d_graphics_settings.h"
#include "hy3d_scene.h"
#include "hy3d_vulkan.h"
#include "camera.h"


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

#define PLATFORM_CHANGE_GRAPHICS(name) bool name(graphics_settings setting, CHANGE_GRAPHICS_SETTINGS newSettings)
typedef PLATFORM_CHANGE_GRAPHICS(platform_change_graphics);

#define VULKAN_PUSH_STAGED_FUNC(name) bool name(staged_resources &)
typedef VULKAN_PUSH_STAGED_FUNC(vulkan_push_staged_func);

struct update_data
{
    // NOTE(heyyod): ENGINE -> VULKAN
    vec3 clearColor;
    
    // NOTE(heyyod): VULKAN -> ENGINE
    // We have multiple a uniform buffer for every frame in the swapchain.
    // We cycle them as we change the current swapchain image.
    // So vulkan will update the pointer and the engine will redirect it's camera write address
    void *newCameraBuffer;
    void *newSceneBuffer;
};

#define VULKAN_DRAW_FUNC(name) bool name(update_data *data)
typedef VULKAN_DRAW_FUNC(vulkan_draw_func);

struct platform_api
{
    vulkan_draw_func *Draw;
    vulkan_push_staged_func *PushStaged;
    
    platform_change_graphics *ChangeGraphicsSettings;
    
    //#if HY3D_DEBUG
    debug_read_file *DEBUGReadFile;
    debug_write_file *DEBUGWriteFile;
    debug_free_file *DEBUGFreeFileMemory;
    //#endif
};

struct camera_data
{
    mat4 view;
    mat4 proj;
};

struct scene_data
{
    vec4 fogColor;        // w for exponent
    vec4 fogDistances;    // x for min, y for max, zw unused
    vec4 ambientColor;
    vec4 sunlightDir;     // w for sun power
    vec4 sunlightColor;
};

struct engine_memory
{
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;
    
    void *stagingMemory;
    void *nextStagingAddr;
    
    // NOTE(heyyod): uniform buffer allocated from vulkan.
    // Linked in os layer for now.
    camera_data *cameraData;
    scene_data *sceneData;
    object_transform *objectsTransforms;
    
    platform_api platformAPI_;
    
    bool isInitialized;
};
global_var platform_api platformAPI;

struct memory_arena
{
    u8 *base;
    u64 size;
    u64 used;
};

enum KEYBOARD_BUTTON
{
    KEY_UP,    KEY_LEFT,  KEY_DOWN,  KEY_RIGHT,
    KEY_W,     KEY_A,     KEY_S,     KEY_D,
    KEY_Q,     KEY_E,     KEY_R,     KEY_F,
    KEY_Z,     KEY_X,     KEY_C,     KEY_V,
    KEY_I,     KEY_J,     KEY_K,     KEY_L, 
    KEY_U,     KEY_O,     KEY_SHIFT, KEY_CTRL,
    KEY_ALT,   KEY_SPACE, KEY_F4,    KEY_TILDE,
    KEY_ZERO,  KEY_ONE,   KEY_TWO,   KEY_THREE, 
    KEY_FOUR,  KEY_FIVE,  KEY_SIX,   KEY_SEVEN, 
    KEY_EIGHT, KEY_NINE,  KEY_COUNT, KEY_INVALID
};

struct keyboard_t
{
    // TODO: USE A SINGLE VARIABLE INSTEAD OF A BOOL ARRAY
    // WE ONLY NEED 1 BIT FOR A KEY
    bool autoRepeatEnabled = false;
    bool isPressed[KEY_COUNT];
    
    bool altWasUp = true;
    
    inline void Clear()
    {
        for (int i = 0; i < KEY_COUNT; i++)
            isPressed[i] = false;
    }
    
    inline void ToggleKey(KEYBOARD_BUTTON key)
    {
        isPressed[key] = !isPressed[key];
    }
};

struct mouse_t
{
    vec2 lastPos;
	vec2 newPos;
    bool isInWindow;
    bool leftIsPressed;
    bool rightIsPressed;
    bool wheelUp;
    bool cursorEnabled;
    
/*    inline void SetPos(vec2 val)
    {
        pos = val;
    }
*/    
    i32 WheelDelta()
    {
        i32 result = wheelDelta;
        wheelDelta = 0;
        return result;
    }
    
    void SetWheelDelta(i32 delta)
    {
		wheelDelta = WheelDelta() + delta;
    }
    
    private:
    i32 wheelDelta;
};

struct engine_input
{
    mouse_t mouse;
    keyboard_t keyboard;
};

struct engine_state
{
    memory_arena memoryArena;
    update_data updateData;
    graphics_settings settings;
    
    camera player;
    
    float time;
};

struct hy3d_engine
{
    engine_input input;
    std::chrono::steady_clock::time_point frameStart;
    bool onResize;
    u32 windowWidth;
    u32 windowHeight;
};

#define UPDATE_AND_RENDER(name) void name(hy3d_engine &e, engine_memory *memory)
typedef UPDATE_AND_RENDER(update_and_render);
UPDATE_AND_RENDER(UpdateAndRenderStub) {}

#endif

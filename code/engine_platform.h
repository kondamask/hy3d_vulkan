#ifndef INLCUDE_ENGINE_PLATFORM_H
#define INLCUDE_ENGINE_PLATFORM_H

//------------------------------------------------------------------------

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH_MIN 255
#define WINDOW_HEIGHT_MIN 255

#define MAX_STAGE_BUFFER_SLOTS 100
#define MAX_TRANSFORM_SLOTS 1000

//------------------------------------------------------------------------

#include <chrono>

#include "core.h"
#include "scene.h"
#include "camera.h"
#include "renderer_platform.h"

//------------------------------------------------------------------------

struct debug_read_file_result;
#define FUNC_READ_FILE(name) debug_read_file_result name(char *filename)
typedef FUNC_READ_FILE(func_read_file);

#define FUNC_WRITE_FILE(name) bool name(char *filename, u32 memorySize, void *memory)
typedef FUNC_WRITE_FILE(func_write_file);

#define FUNC_FREE_FILE(name) void name(void *memory)
typedef FUNC_FREE_FILE(func_free_file);

// NOTE: This just replicates the FILETIME struct from windows
// Should do the same if other platforms are intergrated.
struct file_write_time
{
#ifdef _WIN32
	u32 dwLowDateTime;
	u32 dwHighDateTime;
#endif
};

#define FUNC_GET_FILE_WRITE_TIME(name) bool name(char *filepath, file_write_time *writeTime)
typedef FUNC_GET_FILE_WRITE_TIME(func_get_file_write_time_func);

#define FUNC_WAS_FILE_UPDATED(name) bool name(char *filepath, file_write_time *writeTime)
typedef FUNC_WAS_FILE_UPDATED(func_was_file_updated_func);

//------------------------------------------------------------------------

struct debug_read_file_result
{
    void *content;
    u32 size;
};

struct platform_api
{        
	func_read_file *ReadFile;
    func_write_file *WriteFile;
    func_free_file *FreeFileMemory;
	func_get_file_write_time_func *GetFileWriteTime;
	func_was_file_updated_func *WasFileUpdated;
};

struct engine_memory
{
    u64 permanentMemorySize;
    void *permanentMemory;
    u64 transientMemorySize;
    void *transientMemory;
    
    void *stagingMemory;
    void *nextStagingAddr;
    
    // NOTE(heyyod): uniform buffer allocated from vulkanContext->
    // Linked in os layer for now.
    camera_data *cameraData;
    scene_data *sceneData;
    object_transform *objectsTransforms;
    
    bool isInitialized;
};

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

struct engine_input
{
	struct
	{
		bool autoRepeatEnabled = false;
		bool isPressed[KEY_COUNT]; // TODO: Use a bitfield
    
		bool altWasUp = true;
    
		//------------------------------------------------------------------------
		
		inline void Clear()
		{
			for (int i = 0; i < KEY_COUNT; i++)
				isPressed[i] = false;
		}
    
		inline void ToggleKey(KEYBOARD_BUTTON key)
		{
			isPressed[key] = !isPressed[key];
		}
	} keyboard;

	struct
	{
		vec2 lastPos;
		vec2 newPos;
		bool isInWindow;
		bool leftIsPressed;
		bool rightIsPressed;
		bool wheelUp;
		bool cursorEnabled;
		bool firstMove;
	
		//------------------------------------------------------------------------
		
		i32 GetWheelDelta()
		{
			i32 result = wheelDelta;
			wheelDelta = 0;
			return result;
		}
    
		void UpdateWheelDelta(i32 delta)
		{
			wheelDelta += delta;
		}
    
	private:
		i32 wheelDelta;
	} mouse;
};

// NOTE: This will be a pointer that points to the start of a memory block that we have allocated from the OS.
struct engine_state
{
    memory_arena memoryArena;
    render_packet renderPacket;
    graphics_settings settings;
    
    camera player;
    
    f32 time;

	graphics_context graphicsContext; // NOTE: This must be the last member. We don't know the size. It will be read by the graphics api.
};

struct engine_platform;

#define FUNC_ENGINE_INITIALIZE(name) void name(engine_platform *engine, u32 width, u32 height)
typedef FUNC_ENGINE_INITIALIZE(func_engine_initialize);
FUNC_ENGINE_INITIALIZE(EngineInitializeStub) {}

#define FUNC_ENGINE_UPDATE_AND_RENDER(name) void name(engine_platform *engine)
typedef FUNC_ENGINE_UPDATE_AND_RENDER(func_engine_update_and_render);
FUNC_ENGINE_UPDATE_AND_RENDER(EngineUpdateAndRenderStub) {}

#define FUNC_ENGINE_DESTROY(name) void name(engine_platform *engine)
typedef FUNC_ENGINE_DESTROY(func_engine_destroy);
FUNC_ENGINE_DESTROY(EngineDestroyStub) {}

struct engine_platform
{
	engine_memory memory;
	engine_state *state;
	engine_input input;
	renderer_platform renderer;
	
	file_write_time shadersWriteTime;
	
    std::chrono::steady_clock::time_point frameStart;
    bool onResize;
    u32 windowWidth;
    u32 windowHeight;
	bool reloaded;
	
	//------------------------------------------------------------------------
	// FUNCTIONS
	
	platform_api platformAPI;
	
	func_engine_initialize *Initialize;
	func_engine_update_and_render *UpdateAndRender;
	func_engine_destroy *Destroy;
};

#endif

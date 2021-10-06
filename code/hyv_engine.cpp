#include "hyv_engine.h"
#include "hyv_vulkan.cpp"
#include <intrin.h>

static void InitializeMemoryArena(memory_arena *arena, u8 *base, size_t size)
{
    arena->base = base;
    arena->size = size;
    arena->used = 0;
}

#define ReserveStructMemory(arena, type) (type *)ReserveMemory(arena, sizeof(type))
#define ReserveArrayMemory(arena, count, type) (type *)ReserveMemory(arena, (count) * sizeof(type))
static void *ReserveMemory(memory_arena *arena, size_t size)
{
    Assert(arena->used + size <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

static void Initialize(hyv_engine *e, engine_state *state, engine_memory *memory)
{
    e->input = {};
    e->onResize = false;
    InitializeMemoryArena(&state->memoryArena,
                          (u8 *)memory->permanentMemory + sizeof(engine_state),
                          memory->permanentMemorySize - sizeof(engine_state));

    // NOTE: Everything is initialized here
    state->r = 0.0f;
    state->change = 0.5f;

    memory->isInitialized = true;
    e->frameStart = std::chrono::steady_clock::now();
}

// NOTE: MUST CALL AFTER MEMORY ALLOCATION FROM WINDOWS
extern "C" INIT_VULKAN(InitializeVulkan)
{
    engine_state *state = (engine_state *)memory->permanentMemory;
    if (Vulkan::Win32LoadDLL(state->vulkan))
    {
        return Vulkan::Win32Initialize(state->vulkan, wndInstance, wndHandle, wndName);
    }
    else
    {
        return false;
    }
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    engine_state *state = (engine_state *)memory->permanentMemory;
    if (!memory->isInitialized)
        Initialize(&e, state, memory);

    if (e.onResize)
    {
        if (!Vulkan::OnWindowSizeChange(state->vulkan))
        {
            Assert("Failed To Resize Window.");
            return;
        }
        e.onResize = false;
    }

    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    e.frameStart = frameEnd;

    if (Vulkan::CanRender(state->vulkan))
    {
        // NOTE: UPDATE
        if (state->r <= 0.0f && state->change < 0.0f)
            state->change *= -1.0f;
        else if (state->r >= 0.4f && state->change > 0.0f)
            state->change *= -1.0f;
        state->r += state->change * dt;

        Vulkan::ClearScreen(state->vulkan, state->r);

        // NOTE: RENDER
        Vulkan::Draw(state->vulkan);
    }
    else
    {
        Sleep(100);
    }
}

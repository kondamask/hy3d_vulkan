#include "hy3d_engine.h"
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
    ASSERT(arena->used + size <= arena->size)
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

static void Initialize(hy3d_engine *e, engine_state *state, engine_memory *memory)
{
    e->input = {};
    InitializeMemoryArena(&state->memoryArena,
                          (u8 *)memory->permanentMemory + sizeof(engine_state),
                          memory->permanentMemorySize - sizeof(engine_state));
    memory->isInitialized = true;
    e->frameStart = std::chrono::steady_clock::now();
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
{
    engine_state *state = (engine_state *)memory->permanentMemory;
    if (!memory->isInitialized)
        Initialize(&e, state, memory);

    // NOTE: UPDATE
    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    e.frameStart = frameEnd;

    // NOTE: RENDER
}

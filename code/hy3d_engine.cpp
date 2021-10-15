#include "hy3d_engine.h"
#include <intrin.h>

function void InitializeMemoryArena(memory_arena *arena, u8 *base, size_t size)
{
    arena->base = base;
    arena->size = size;
    arena->used = 0;
}

#define ReserveStructMemory(arena, type) (type *)ReserveMemory(arena, sizeof(type))
#define ReserveArrayMemory(arena, count, type) (type *)ReserveMemory(arena, (count) * sizeof(type))
function void *ReserveMemory(memory_arena *arena, size_t size)
{
    Assert(arena->used + size <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

function void Initialize(hy3d_engine *e, engine_state *state, engine_memory *memory)
{
    e->input = {};
    e->onResize = false;
    InitializeMemoryArena(&state->memoryArena,
                          (u8 *)memory->permanentMemory + sizeof(engine_state),
                          memory->permanentMemorySize - sizeof(engine_state));
    
    // NOTE: Everything is initialized here
    state->updateData.clearColor[0] = 0.1f;
    state->updateData.clearColor[1] = 0.3f;
    state->updateData.clearColor[2] = 0.6f;
    
    mesh &m = state->updateData.testMesh;
    
    m.nVertices = 4;
    m.nIndices = 6;
    m.vertices = (vertex *)memory->stagingMemory;
    m.indices = (index *)(&m.vertices[m.nVertices]);
    m.vertices[0] = { {-0.5f, -0.5f}, {1.0f, 1.0f, 0.0f} };
    m.vertices[1] = { { 0.5f, -0.5f}, {0.0f, 1.0f, 1.0f} };
    m.vertices[2] = { { 0.5f,  0.5f}, {1.0f, 0.0f, 1.0f} };
    m.vertices[3] = { {-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f} };
    m.indices[0] = 0;
    m.indices[1] = 1;
    m.indices[2] = 2;
    m.indices[3] = 2;
    m.indices[4] = 3;
    m.indices[5] = 0;
    
    state->updateData.updateVertexBuffer = true;
    
    state->clearColorChange[0] = 1.0f;
    state->clearColorChange[1] = 1.5f;
    state->clearColorChange[2] = 2.0f;
    
    memory->isInitialized = true;
    e->frameStart = std::chrono::steady_clock::now();
}

extern "C"
UPDATE_AND_RENDER(UpdateAndRender)
{
    platformAPI = memory->platformAPI_;
    engine_state *state = (engine_state *)memory->permanentMemory;
    
    if (!memory->isInitialized)
        Initialize(&e, state, memory);
    
    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    e.frameStart = frameEnd;
    
    // NOTE: UPDATE
    f32 min = 0.5f;
    f32 max = 1.0f;
    for (u8 i = 0; i < ArrayCount(state->updateData.clearColor); i++)
    {
        if (state->updateData.clearColor[i] >= max)
        {
            state->clearColorChange[i] *= -1.0f;
            state->updateData.clearColor[i] = max;
        }
        if (state->updateData.clearColor[i] <= min)
        {
            state->clearColorChange[i] *= -1.0f;
            state->updateData.clearColor[i] = min;
        }
        state->updateData.clearColor[i] += state->clearColorChange[i] * dt * 1.3f;
    }
    
    platformAPI.Draw(&state->updateData);
}
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
    state->updateData.verts[0].pos = {-1.0f, 1.0f };
    state->updateData.verts[1].pos = { 0.0f, -1.0 };
    state->updateData.verts[2].pos = { 1.0f, 1.0f };
    state->updateData.verts[0].color = {1.0f, 0.0f, 0.0f};
    state->updateData.verts[1].color = {0.0f, 1.0f, 0.0f};
    state->updateData.verts[2].color = {0.0f, 0.0f, 1.0f};
    
    state->clearColorChange[0] = 1.0f;
    state->clearColorChange[1] = 1.5f;
    state->clearColorChange[2] = 2.0f;
    
    memory->isInitialized = true;
    e->frameStart = std::chrono::steady_clock::now();
}

extern "C" UPDATE_AND_RENDER(UpdateAndRender)
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
    float min = 0.0f;
    float max = 1.0f;
    for (int i = 0; i < ArrayCount(state->updateData.clearColor); i++)
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
        state->updateData.clearColor[i] += state->clearColorChange[i] * dt * 0.3f;
    }
    state->updateData.verts[0].pos.x = -state->updateData.clearColor[1];
    state->updateData.verts[0].pos.y = state->updateData.clearColor[0];
    state->updateData.verts[1].pos.x =  state->updateData.clearColor[0];
    state->updateData.verts[1].pos.y = -state->updateData.clearColor[2];
    state->updateData.verts[2].pos.x = state->updateData.clearColor[2];
    state->updateData.verts[2].pos.y = state->updateData.clearColor[1];
    state->updateData.verts[0].color.pos[0] = state->updateData.clearColor[0];
    state->updateData.verts[1].color.pos[1] = state->updateData.clearColor[1];
    state->updateData.verts[2].color.pos[2] = state->updateData.clearColor[2];
    
    
    //state->color[0] = 0.8;
    //state->color[1] = 0.55;
    //state->color[2] = 0.35;
    
    platformAPI.Update(&state->updateData);
    
    // NOTE: RENDER
    platformAPI.Draw();
    
    //if we cant render
    //Sleep(100);
}
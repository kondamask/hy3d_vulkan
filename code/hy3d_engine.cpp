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

extern "C"
UPDATE_AND_RENDER(UpdateAndRender)
{
    engine_state *state = (engine_state *)memory->permanentMemory;
    update_data &update = state->updateData;
    platformAPI = memory->platformAPI_;
    
    if (!memory->isInitialized)
    {
        e.input = {};
        e.onResize = false;
        InitializeMemoryArena(&state->memoryArena,
                              (u8 *)memory->permanentMemory + sizeof(engine_state),
                              memory->permanentMemorySize - sizeof(engine_state));
        
        // NOTE: Everything is initialized here
        state->updateData.clearColor[0] = 0.1f;
        state->updateData.clearColor[1] = 0.3f;
        state->updateData.clearColor[2] = 0.6f;
        
        memory->nextStagingAddr = memory->stagingMemory;
        
        mesh *m = state->updateData.meshes;
        m[0].nVertices = 4;
        m[0].nIndices = 6;
        m[0].vertices = (vertex *)memory->nextStagingAddr;
        m[0].indices = (index *)(MESH_VERTICES_END_ADDR(m[0]));
        m[0].vertices[0] = { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} };
        m[0].vertices[1] = { { 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} };
        m[0].vertices[2] = { { 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f} };
        m[0].vertices[3] = { {-0.5f,  0.5f}, {0.0f, 0.0f, 0.0f} };
        m[0].indices[0] = 0;
        m[0].indices[1] = 1;
        m[0].indices[2] = 2;
        m[0].indices[3] = 2;
        m[0].indices[4] = 3;
        m[0].indices[5] = 0;
        
        // NOTE(heyyod): I'll have something like LoadMesh(...) and it will also update the 
        // nextStagingAddr
        /*memory->nextStagingAddr = (u8 *)memory->stagingMemory + MESH_TOTAL_SIZE(m[0]);
        
        m[1].nVertices = 4;
        m[1].nIndices = 6;
        m[1].vertices = (vertex *)memory->nextStagingAddr;
        m[1].indices = (index *)(MESH_VERTICES_END_ADDR(m[1]));
        m[1].vertices[0] = { {-0.8f, -0.8f}, {1.0f, 1.0f, 0.0f} };
        m[1].vertices[1] = { {-0.3f, -0.8f}, {0.0f, 1.0f, 1.0f} };
        m[1].vertices[2] = { {-0.3f,  0.0f}, {1.0f, 0.0f, 1.0f} };
        m[1].vertices[3] = { {-0.8f,  0.0f}, {1.0f, 1.0f, 1.0f} };
        m[1].indices[0] = 0;
        m[1].indices[1] = 1;
        m[1].indices[2] = 2;
        m[1].indices[3] = 2;
        m[1].indices[4] = 3;
        m[1].indices[5] = 0;*/
        
        state->updateData.updateVertexBuffer = true;
        
        state->clearColorChange[0] = 1.0f;
        state->clearColorChange[1] = 1.5f;
        state->clearColorChange[2] = 2.0f;
        
        memory->isInitialized = true;
        state->time = 0;
        state->camPos = {0.0f, 0.0f, 2.0f};
        e.frameStart = std::chrono::steady_clock::now();
    }
    
    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    state->time += dt;
    e.frameStart = frameEnd;
    
    // NOTE: UPDATE
    f32 min = 0.5f;
    f32 max = 1.0f;
    for (u8 i = 0; i < ArrayCount(state->updateData.clearColor); i++)
    {
        if (update.clearColor[i] >= max)
        {
            state->clearColorChange[i] *= -1.0f;
            update.clearColor[i] = max;
        }
        if (update.clearColor[i] <= min)
        {
            state->clearColorChange[i] *= -1.0f;
            update.clearColor[i] = min;
        }
        update.clearColor[i] += state->clearColorChange[i] * dt * 1.3f;
    }
    
    f32 zChange = 0.0f;
    if (e.input.keyboard.isPressed[KEY_Z])
    {
        zChange = dt * 2.0f;
    }
    if (e.input.keyboard.isPressed[KEY_X])
    {
        zChange = - dt * 2.0f;
    }
    state->camPos.Z += zChange;
    
    
    memory->mvp->model = Rotate(state->time * 90.0f, Vec3(1.0f, 0.0f, 0.0f));
    memory->mvp->view = LookAt(state->camPos, Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    memory->mvp->proj = Perspective(45.0f, e.windowWidth / (f32) e.windowHeight, 0.1f, 10.0f);
    //memory->mvp->proj[1][1] *= -1.0f;
    platformAPI.Draw(&state->updateData);
    
    // NOTE(heyyod): Update stuff from vulkan
    memory->mvp = (model_view_proj *)state->updateData.newMVP;
}
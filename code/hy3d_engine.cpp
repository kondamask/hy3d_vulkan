#include "hy3d_engine.h"
#include <intrin.h>

function void
InitializeMemoryArena(memory_arena *arena, u8 *base, size_t size)
{
    arena->base = base;
    arena->size = size;
    arena->used = 0;
}

#define ReserveStructMemory(arena, type) (type *)ReserveMemory(arena, sizeof(type))
#define ReserveArrayMemory(arena, count, type) (type *)ReserveMemory(arena, (count) * sizeof(type))
function void *
ReserveMemory(memory_arena *arena, size_t size)
{
    Assert(arena->used + size <= arena->size);
    void *result = arena->base + arena->used;
    arena->used += size;
    return result;
}

#define OffsetInStageBuffer(res) (u8*)res - (u8*)stagedResources.resources[0] + sizeof(*res)
function bool 
StageResource(const char *filepath, RESOURCE_TYPE type, staged_resources &stagedResources)
{
    u32 bytesStaged = 0;
    
    if (stagedResources.count == 0)
        stagedResources.resources[0] = stagedResources.nextWriteAddr;
    
    switch (type)
    {
        case RESOURCE_MESH:
        {
            mesh *m = (mesh *)stagedResources.nextWriteAddr;
            if (!LoadOBJ(filepath, m))
                return false;
            stagedResources.offsets[stagedResources.count] = OffsetInStageBuffer(m);
            bytesStaged = MESH_PTR_TOTAL_SIZE(m);
        }break;
        
        case RESOURCE_TEXTURE:
        {
            image *img = (image *)stagedResources.nextWriteAddr;
            if (!LoadImageRGBA(filepath, img))
                return false;
            stagedResources.offsets[stagedResources.count] = OffsetInStageBuffer(img); 
            bytesStaged = IMAGE_TOTAL_SIZE((*img));
        }break;
        
        default:
        {
            stagedResources.types[stagedResources.count] = RESOURCE_INVALID;
            DebugPrint("ERROR: Trying to stage unknown resource type.\n");
            return false;
        }
    }
    stagedResources.resources[stagedResources.count] = stagedResources.nextWriteAddr;
    stagedResources.types[stagedResources.count] = type;
    stagedResources.count++;
    AdvancePointer(stagedResources.nextWriteAddr, bytesStaged);
    return true;
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
        
        {
            staged_resources resources = {};
            resources.nextWriteAddr = memory->stagingMemory;
            StageResource("../models/viking_room.obj", RESOURCE_MESH, resources);
            //StageResource("../models/test.obj", RESOURCE_MESH, resources);
            StageResource("../textures/viking_room.png", RESOURCE_TEXTURE, resources);
            /* 
            StageResource("../models/cube.obj", RESOURCE_MESH, resources);
            StageResource("../textures/default.png", RESOURCE_TEXTURE, resources);
            StageResource("../models/bunny.obj", RESOURCE_MESH, resources);
            StageResource("../textures/bunny_tex.bmp", RESOURCE_TEXTURE, resources);
                         */
            
            platformAPI.PushStaged(resources);
        }
        
        state->clearColorChange[0] = 1.0f;
        state->clearColorChange[1] = 1.5f;
        state->clearColorChange[2] = 2.0f;
        
        memory->isInitialized = true;
        state->time = 0.0f;
        state->camTheta = ToRadians(-90.0f);
        state->radius = 5.0f;
        state->camPos.X = state->radius * CosF(state->camTheta);
        state->camPos.Y = 1.0f;
        state->camPos.Z = state->radius * SinF(state->camTheta);
        e.frameStart = std::chrono::steady_clock::now();
    }
    
    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    state->time += dt;
    e.frameStart = frameEnd;
    
    // NOTE: UPDATE
    f32 min = 0.2f;
    f32 max = 0.6f;
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
        update.clearColor[i] += state->clearColorChange[i] * dt * 0.1f;
    }
    
    if(!e.input.keyboard.isPressed[KEY_CTRL] && !e.input.keyboard.ctrlWasUp)
        e.input.keyboard.ctrlWasUp = true;
    if(e.input.keyboard.isPressed[KEY_CTRL] && e.input.keyboard.ctrlWasUp)
    {
        
        CHANGE_GRAPHICS_SETTINGS newSettings = CHANGE_NONE;
        MSAA_OPTIONS newMSAA = state->settings.msaa;
        
        if(e.input.keyboard.isPressed[KEY_ONE])
        {
            newMSAA = MSAA_OFF;
            e.input.keyboard.ctrlWasUp = false;
        }
        if(e.input.keyboard.isPressed[KEY_TWO])
        {
            newMSAA = MSAA_2;
            e.input.keyboard.ctrlWasUp = false;
        }
        if(e.input.keyboard.isPressed[KEY_THREE])
        {
            newMSAA = MSAA_4;
            e.input.keyboard.ctrlWasUp = false;
        }
        if(e.input.keyboard.isPressed[KEY_FOUR])
        {
            newMSAA = MSAA_8;
            e.input.keyboard.ctrlWasUp = false;
        }
        if (newMSAA != state->settings.msaa)
        {
            state->settings.msaa = newMSAA;
            newSettings |= CHANGE_MSAA;
        }
        
        if (newSettings)
            platformAPI.ChangeGraphicsSettings(state->settings, newSettings);
    }
    
    f32 camSpeed = 0.1f;
    if (e.input.keyboard.isPressed[KEY_Z])
        state->radius -= dt * camSpeed * 2.0f;
    if (e.input.keyboard.isPressed[KEY_X])
        state->radius += dt * camSpeed * 2.0f;
    
    if (e.input.keyboard.isPressed[KEY_RIGHT])
        state->camTheta += dt * camSpeed;
    if (e.input.keyboard.isPressed[KEY_LEFT])
        state->camTheta -= dt * camSpeed;
    
    state->camPos.X = state->radius * CosF(state->camTheta);
    state->camPos.Z = state->radius * SinF(state->camTheta);
    
    //vec3 pos = {};
    //pos.X = SinF(2.0f * state->time);
    //pos.Z = 6.0f;
    //pos.Y = CosF(state->time);
    
    //f32 scale = 7.0f;
    memory->mvp->model =
        Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
        Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f)) *
        //Scale({scale, scale, scale}) *
        Translate({0.0f, 0.0f, 0.0f})
        ;
    memory->mvp->view = LookAt({state->camPos.X, 0.3f, state->camPos.Z}, {0.0f, 0.3f,0.0f}, Vec3(0.0f, -1.0f, 0.0f));
    memory->mvp->proj = Perspective(45.0f, e.windowWidth / (f32) e.windowHeight, 0.1f, 10.0f);
    //memory->mvp->proj[1][1] *= -1.0f;
    platformAPI.Draw(&state->updateData);
    
    // NOTE(heyyod): Update stuff from vulkan
    memory->mvp = (model_view_proj *)state->updateData.newMvpBuffer;
}

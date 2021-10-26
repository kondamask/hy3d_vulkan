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
            StageResource("../textures/viking_room.png", RESOURCE_TEXTURE, resources);
            /* 
            StageResource("../models/Lev-edinorog_complete.obj", RESOURCE_MESH, resources);
            StageResource("../textures/Lev-edinorog_complete_0.png", RESOURCE_TEXTURE, resources);
            StageResource("../models/cube.obj", RESOURCE_MESH, resources);
            StageResource("../textures/default.png", RESOURCE_TEXTURE, resources);
            StageResource("../models/bunny.obj", RESOURCE_MESH, resources);
            StageResource("../textures/bunny_tex.bmp", RESOURCE_TEXTURE, resources);
                         */
            
            platformAPI.PushStaged(resources);
        }
        
        memory->isInitialized = true;
        
        
        memory->sceneData->ambientColor = {0.5f,0.0f,0.0f,0.0f};
        
        
        state->player.pos = {0.0f, 2.0f, -3.0f};
        state->player.lookDir = {0.0f, -0.2f, 1.0f, 0.0f};
        state->player.moveSpeed = 0.7f;
        state->player.lookSens = 0.02f;
        state->player.fov = 50.0f;
        state->player.lookSens = 100.0f;
        
        e.input.mouse.cursorEnabled = false;
        
        state->time = 0.0f;
        e.frameStart = std::chrono::steady_clock::now();
    }
    
    // NOTE(heyyod): Frame time
    std::chrono::steady_clock::time_point frameEnd = std::chrono::steady_clock::now();
    std::chrono::duration<f32> frameTime = frameEnd - e.frameStart;
    f32 dt = frameTime.count();
    state->time += dt;
    e.frameStart = frameEnd;
    
    // NOTE(heyyod): SETTINGS CONTROL
    update.clearColor = {0.7f, 0.4f, 0.3f};
    
    {
        if(!e.input.keyboard.isPressed[KEY_ALT] && !e.input.keyboard.altWasUp)
            e.input.keyboard.altWasUp = true;
        if(e.input.keyboard.isPressed[KEY_ALT] && e.input.keyboard.altWasUp)
        {
            
            CHANGE_GRAPHICS_SETTINGS newSettings = CHANGE_NONE;
            MSAA_OPTIONS newMSAA = state->settings.msaa;
            
            if(e.input.keyboard.isPressed[KEY_ONE])
            {
                newMSAA = MSAA_OFF;
            }
            if(e.input.keyboard.isPressed[KEY_TWO])
            {
                newMSAA = MSAA_2;
            }
            if(e.input.keyboard.isPressed[KEY_THREE])
            {
                newMSAA = MSAA_4;
            }
            if(e.input.keyboard.isPressed[KEY_FOUR])
            {
                newMSAA = MSAA_8;
            }
            if (newMSAA != state->settings.msaa)
            {
                state->settings.msaa = newMSAA;
                newSettings |= CHANGE_MSAA;
                e.input.keyboard.altWasUp = false;
            }
            if (newSettings)
                platformAPI.ChangeGraphicsSettings(state->settings, newSettings);
            
            if (e.input.keyboard.isPressed[KEY_TILDE])
            {
                e.input.mouse.cursorEnabled = !e.input.mouse.cursorEnabled;
                e.input.keyboard.altWasUp = false;
                //platformAPI.SetCursorMode(e.input.mouse.cursorEnabled);
            }
        }
    }
    
    // NOTE(heyyod): CAMERA CONTROL
    {
        camera &player = state->player;
        vec3 moveDir = {};
        bool blockRight = false;
        if(e.input.keyboard.isPressed[KEY_W])
        {
            moveDir += player.lookDir.XYZ;
        }
        if(e.input.keyboard.isPressed[KEY_S])
        {
            moveDir -= player.lookDir.XYZ;
        }
        if(e.input.keyboard.isPressed[KEY_A])
        {
            if (e.input.keyboard.isPressed[KEY_D])
                blockRight = true;
            moveDir += Cross(VULKAN_UP, player.lookDir.XYZ);
        }
        if(e.input.keyboard.isPressed[KEY_D] && !blockRight)
        {
            moveDir += Cross(player.lookDir.XYZ, VULKAN_UP);
        }
        if(e.input.keyboard.isPressed[KEY_CTRL])
        {
            moveDir.Y -= 1.0f;
        }
        if(e.input.keyboard.isPressed[KEY_SPACE])
        {
            moveDir.Y += 1.0f;
        }
        player.pos += NormalizeVec3(moveDir) * player.moveSpeed * 2.0f * dt;
        
        // TODO(heyyod): FIX THE CAMERA AT VERTICAL +-90deg
        if (e.input.mouse.cursorEnabled)
            e.input.mouse.lastPos = e.input.mouse.delta;
        if (!e.input.mouse.cursorEnabled)
        {
            vec2 dir = ((e.input.mouse.delta - e.input.mouse.lastPos) * player.lookSens * 0.05f * dt);
            e.input.mouse.lastPos = e.input.mouse.delta;
            
            if (dir.X != 0.0f)
                player.lookDir = Rotate(dir.X, {0.0f, 1.0f, 0.0f}) * player.lookDir;
            if (dir.Y != 0.0f)
                player.lookDir = Rotate(dir.Y, {player.lookDir.Z, 0.0f, -player.lookDir.X}) * player.lookDir;
        }
    }
    
    /*
    f32 scale = 1.0f;
    memory->cameraData->model =
        Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
        Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f)) *
        Scale({scale, scale, scale}) *
        Translate({0.0f, 0.0f, 0.0f})
        ;
*/
    memory->objectsData[0].model = 
        Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
        Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f)) *
        Translate({0.0f, 0.0f, 0.0f});
    memory->cameraData->view = LookAt(state->player.pos, state->player.pos + state->player.lookDir.XYZ, VULKAN_UP);
    memory->cameraData->proj = Perspective(state->player.fov, e.windowWidth / (f32) e.windowHeight, 0.1f, 10.0f);
    memory->cameraData->proj[1][1] *= -1.0f;
    memory->sceneData->ambientColor.X = 0.5f;
    memory->sceneData->ambientColor.Y = 0.7f;
    memory->sceneData->ambientColor.Z = 0.8f;
    
    DebugPrint(memory->sceneData->ambientColor.X);
    DebugPrint('\n');
    platformAPI.Draw(&state->updateData);
    
    // NOTE(heyyod): Update stuff from vulkan
    memory->cameraData = (camera_data *)update.newCameraBuffer;
    memory->objectsData = (object_data *)update.newObjectsBuffer;
    memory->sceneData= (scene_data *)update.newSceneBuffer;
}

/* date = October 28th 2021 5:43 pm */

#ifndef HY3D_SCENE_H
#define HY3D_SCENE_H

#define STB_DS_IMPLEMENTATION
#include "libs/stb_ds.h"

enum RESOURCE_TYPE
{
    RESOURCE_EMPTY,
    
    RESOURCE_MESH,
    RESOURCE_TEXTURE,
    
    RESOURCE_INVALID
};

struct object_transform
{
    // TODO(heyyod): This can hold rotation, scale etc
    mat4 model;
};

struct staged_resources
{
    void *nextWriteAddr;
    void *resources[MAX_STAGE_BUFFER_SLOTS];
    u64 offsets[MAX_STAGE_BUFFER_SLOTS]; //bytes from the start of the staging buffer
    RESOURCE_TYPE types[MAX_STAGE_BUFFER_SLOTS];
    u32 nInstances[MAX_STAGE_BUFFER_SLOTS];
    object_transform *transforms;
    u32 count;
};

#define OffsetInStageBuffer(res) (u8*)res - (u8*)sceneResources.resources[0] + sizeof(*res)
function_ i32
StageResource(const char *filepath, RESOURCE_TYPE type, staged_resources &sceneResources)
{
    u32 bytesStaged = 0;
    
    if (sceneResources.count == 0)
        sceneResources.resources[0] = sceneResources.nextWriteAddr;
    
    switch (type)
    {
        case RESOURCE_MESH:
        {
            mesh *m = (mesh *)sceneResources.nextWriteAddr;
            if (!LoadOBJ(filepath, m))
                return -1;
            sceneResources.offsets[sceneResources.count] = OffsetInStageBuffer(m);
            bytesStaged = MESH_PTR_TOTAL_SIZE(m);
        }break;
        
        case RESOURCE_TEXTURE:
        {
            image *img = (image *)sceneResources.nextWriteAddr;
            if (!LoadImageRGBA(filepath, img))
                return -1;
            sceneResources.offsets[sceneResources.count] = OffsetInStageBuffer(img); 
            bytesStaged = IMAGE_TOTAL_SIZE((*img));
        }break;
        
        default:
        {
            sceneResources.types[sceneResources.count] = RESOURCE_INVALID;
            DebugPrint("ERROR: Trying to stage unknown resource type.\n");
            return -1;
        }
    }
    sceneResources.resources[sceneResources.count] = sceneResources.nextWriteAddr;
    sceneResources.types[sceneResources.count] = type;
    sceneResources.count++;
    AdvancePointer(sceneResources.nextWriteAddr, bytesStaged);
    return (sceneResources.count - 1);
}


#define IsResourceValid(id) (id >= 0)
function_ staged_resources
CreateScene(void *stageMemory)
{
    staged_resources resources = {};
    resources.nextWriteAddr = stageMemory;
    i32 resourceID = -1;
    resourceID = StageResource("../models/viking_room.obj", RESOURCE_MESH, resources);
    if(IsResourceValid(resourceID))
    {
        object_transform t0;
        t0.model = 
            Translate({0.0f, 0.0f, 0.0f}) *
            Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
            Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f));
        object_transform t1;
        t1.model = 
            Translate({0.0f, 2.0f, 0.0f}) *
            Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
            Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f));
        arrput(resources.transforms, t0);
        arrput(resources.transforms, t1);
        resources.nInstances[resourceID] = 2;
    }
    
    // TODO(heyyod): MUST CLEAR ARRAY AFTER PUSH!!!!!
    StageResource("../textures/viking_room.png", RESOURCE_TEXTURE, resources);
    
    return resources;
}

#endif //HY3D_SCENE_H

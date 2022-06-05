#include "scene.h"
/*
static_func i32 StageResource(const char *filepath, RESOURCE_TYPE type, scene_resources &resources)
{
    u32 bytesStaged = 0;

    if (resources.count == 0)
        resources.resources[0] = resources.nextWriteAddr;

    switch (type)
    {
        case RESOURCE_MESH:
        {
            mesh *m = (mesh *)resources.nextWriteAddr;
            if (!LoadOBJ(filepath, m))
                return -1;
            resources.offsets[resources.count] = OffsetInStageBuffer(m);
            bytesStaged = MESH_PTR_TOTAL_SIZE(m);
        } break;

        case RESOURCE_TEXTURE:
        {
            image *img = (image *)resources.nextWriteAddr;
            if (!LoadImageRGBA(filepath, img))
                return -1;
            resources.offsets[resources.count] = OffsetInStageBuffer(img);
            bytesStaged = IMAGE_TOTAL_SIZE((*img));
        } break;

        default:
        {
            resources.types[resources.count] = RESOURCE_INVALID;
            DebugPrint("ERROR: Trying to stage unknown resource type.\n");
            return -1;
        }
    }
    resources.resources[resources.count] = resources.nextWriteAddr;
    resources.types[resources.count] = type;
    resources.count++;
    AdvancePointer(resources.nextWriteAddr, bytesStaged);
    return (resources.count - 1);
}
*/
// NOTE(heyyod): 
/*
I can probably have a hash map of all the loaded resources.
Also every scene can hold the names of the resources it uses.
Using the hash map I can check which resources are already loaded into the gpu.
Then if I need to load more resources that are not currently available I can
a) Allocate vram if available
b) Free unecesary resources from older scenes (LRU?)
This WILL cause unused gaps in the buffer so I need to keep this in mind!!
*/

#define OffsetInStageBuffer(ptr) (u8*)(ptr) - (u8*)(assets[0].header)
static_func i32 LoadModel(char *modelPath, char *texturePath, u32 instances, void *&buffer, asset *assets, u32 &count, scene_data &scene)
{
    assets[count].header = buffer;
    if (!LoadOBJ(modelPath, &assets[count]))
        return -1;
    mesh_header *m = (mesh_header *)&assets[count];
    if (m->nIndices)
        m->indicesOffset = OffsetInStageBuffer(m->indices);
    m->verticesOffset = OffsetInStageBuffer(m->vertices);
    count++;
    AdvancePointer(buffer, MESH_PTR_TOTAL_SIZE(m));

    if (texturePath)
    {
        assets[count].header = buffer;
        if (!LoadImageRGBA(texturePath, &assets[count]))
            return -1;
        image_header *img = (image_header *)&assets[count];
        img->offset = OffsetInStageBuffer(img->pixels);
        count++;
        AdvancePointer(buffer, IMAGE_TOTAL_SIZE((*img)));
        
        scene.loadedMeshes[scene.loadedMeshesCount].textureIndex = scene.loadedTexturesCount++;
    }

    scene.loadedMeshes[scene.loadedMeshesCount].instances = instances;
    scene.loadedMeshes[scene.loadedMeshesCount].nIndices = m->nIndices;
    scene.loadedMeshes[scene.loadedMeshesCount].nVertices = m->nVertices;
    scene.loadedMeshesCount++;
    
    Assert(scene.loadedMeshesCount - 1 >= 0);
    return (scene.loadedMeshesCount - 1);
}


#define IsResourceValid(id) (id >= 0)
static_func void CreateScene(scene_data &scene, scene_ubo &sceneUBO)
{
    // TODO: Remove the vulkan related stuff
    void *buffer = VulkanRequestBuffer(RENDERER_BUFFER_TYPE_STAGING, MEGABYTES(64), 0);
    u64 offset = 0;

    asset assets[10] = {};
    u32 count = 0;
    LoadModel(MODEL_PATH("viking_room.obj"), TEXTURE_PATH("viking_room.png"), 3, buffer, assets, count, scene);
    LoadModel(MODEL_PATH("Lev-edinorog_complete.obj"), TEXTURE_PATH("Lev-edinorog_complete.png"), 1, buffer, assets, count, scene);
    LoadModel(MODEL_PATH("cube.obj"), TEXTURE_PATH("default.png"), 1, buffer, assets, count, scene);
    
    VulkanUpload(assets, 1);
    VulkanClearBuffer(vulkanContext->stagingBuffer);

    sceneUBO.ambientColor = { 0.5f, 0.5f, 0.65f, 0.0f };
}

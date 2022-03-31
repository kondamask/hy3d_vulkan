/* date = October 28th 2021 5:43 pm */

#ifndef HY3D_SCENE_H
#define HY3D_SCENE_H

#define MODEL_PATH(file) ".\\models\\"#file

#define TEXTURE_PATH(file) ".\\textures\\"#file

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
    u32 count;
};

#define OffsetInStageBuffer(res) (u8*)res - (u8*)sceneResources.resources[0] + sizeof(*res)
static_func i32
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


#define IsResourceValid(id) (id >= 0)
static_func staged_resources
CreateScene(staged_resources &resources, void *transforms)
{
    // TODO(heyyod): Staging buffer shoulb be created here
    i32 resourceID = -1;
    resourceID = StageResource(MODEL_PATH(viking_room.obj), RESOURCE_MESH, resources);
    if(IsResourceValid(resourceID))
    {
        object_transform *t = (object_transform *)transforms;
        t[0].model = 
            Translate({0.0f, 0.0f, 0.0f}) *
            Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
            Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f));
        t[1].model = 
            Translate({0.0f, 2.0f, 0.0f}) *
            Rotate(90.0f, Vec3(0.0f, 0.0f, 1.0f)) *
            Rotate(90.0f, Vec3(0.0f, 1.0f, 0.0f));
        //arrput(resources.transforms, t0);
        //arrput(resources.transforms, t1);
        resources.nInstances[resourceID] = 2;
    }
    StageResource(TEXTURE_PATH(viking_room.png), RESOURCE_TEXTURE, resources);
    return resources;
}

#endif //HY3D_SCENE_H

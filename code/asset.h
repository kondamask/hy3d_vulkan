#ifndef INCLUDE_ASSET_H
#define INCLUDE_ASSET_H

enum ASSET_TYPE_TAG
{
    ASSET_TYPE_INVALID, // Assets must be initialize we a type other than 0. This makes it easier to detect uninitialized assets

    ASSET_TYPE_MESH,
    ASSET_TYPE_TEXTURE
};

/**
 * The asset pipeline:
 * Let's say we need to load a mesh. This has a number of vertices, and probably indices.
 * The vertices and indices are initialy saved in a staging buffer at a certain offset.
 * So for every mesh we need to know:
 *  1) How many vertices and indices it has so that we can calc the total size of each set
 *  2) A pointer to the vertices and indices in that buffer
 * 
 * This can be translated to an asset being consisted of a header and its actual data.
 * We need a header because different asset need different info about how to read/copy their data.
 * */

struct asset
{
    ASSET_TYPE_TAG type;
    void *header; 
};
#endif
//
//  Scene.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 9/23/22.
//

#ifndef Scene_h
#define Scene_h

#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <vector>

typedef struct {
    
    // all of these arrays should be the same size
    std::vector<size_t> modelIndices; // which objects correspond to which models
    std::vector<size_t> vertexOffsets;
    std::vector<size_t> indexOffsets;
    
    size_t objectCount; // the number of objects in the scene
    
    MTL::Texture* textures[1000];
    MTL::Buffer *vertices;
    MTL::Buffer *indices;
    
    // this should be the transform of the object at the start of the scene? So starting position, orientation, etc
    std::vector<matrix_float4x4> transforms; // this will contain the mvp matrix for every model in the scene?
    std::vector<vector_float4> colors;
    
    bool isLoaded; // one thing I don't like about this is that it could be wrong
    
    // should this be like, more than enough, right
    const char* rawModelNames[1000];
    const char* rawTextureNames[1000];
} Scene;

#endif /* Scene_h */

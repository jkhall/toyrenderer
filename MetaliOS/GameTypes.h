//
//  GameTypes.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 8/27/22.
//

#ifndef GameTypes_h
#define GameTypes_h

#include <simd/simd.h>

// i guess a Thing should really be concerned with only position right
// and id, maybe

// should all meshes be kept in one massive array?

typedef struct {
    int ID = -1;
    
    matrix_float4x4 modelView = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0}
    };
    
    float4 color = {0, 0, 0, 1};
    
    int meshId = -1; // could look up into a set of starts and stops    
    
    int textureID = -1; // this will be a lookup for
    
    // should i put texture's here
} Thing;

/*
 What's the size of the gallery
 Should 
 */

typedef struct {
    bool showingGallery;
    bool takingPicture;
} GameState;


#endif /* GameTypes_h */

//
//  Physics.hpp
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 10/1/22.
//

#ifndef Physics_hpp
#define Physics_hpp

#include "Scene.hpp"
#include <stdio.h>
#include <simd/simd.h>

class Physics {
public:
    Physics(Scene *scene);
    ~Physics();
    matrix_float4x4 tick(size_t modelIndex);
private:
    Scene *_pScene;
};

#endif /* Physics_hpp */

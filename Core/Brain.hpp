//
//  Brain.hpp
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 10/1/22.
//

#ifndef Brain_hpp
#define Brain_hpp

#include "Scene.hpp"
#include <stdio.h>
#include <simd/simd.h>

class Brain {
public:
    Brain(Scene *scene);
    ~Brain();
    matrix_float4x4 tick(size_t modelIndex);
private:
    Scene *_pScene;
};

#endif /* Brain_hpp */

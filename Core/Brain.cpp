//
//  Brain.cpp
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 10/1/22.
//

#include "Brain.hpp"

#include <simd/simd.h>

Brain::Brain(Scene *scene) {
    _pScene = scene;
}

Brain::~Brain() {
    
}

matrix_float4x4 Brain::tick(size_t modelIndex) {
    return matrix_identity_float4x4;
}


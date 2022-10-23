//
//  Physics.cpp
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 10/1/22.
//

#include "Physics.hpp"

Physics::Physics(Scene *scene) {
    
}

Physics::~Physics() {
    
}

// shouldn't this return a matrix?
matrix_float4x4 Physics::tick(size_t modelIndex) {
    return matrix_identity_float4x4;
}

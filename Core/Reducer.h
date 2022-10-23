//
//  Reducer.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 9/30/22.
//

#ifndef Reducer_h
#define Reducer_h

#include "Scene.hpp"
#include "Brain.hpp"
#include "Physics.hpp"
#include "AAPLShaderTypes.h"
#include <simd/simd.h>

// have to figure out how to return the series of matrices that we need
// can't include the renderer here

@interface Reducer : NSObject
    - (matrix_float4x4) tick:(size_t)objectIndex;
    @property Scene *scene;
    @property Brain *brain;
    @property Physics *physics;
    - (id) initWithScene:(Scene *)scene;
@end



#endif /* Reducer_h */

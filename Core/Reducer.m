//
//  Reducer.m
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 9/30/22.
//

#import <Foundation/Foundation.h>
#import <simd/simd.h>
#import "Reducer.h"
#import "Scene.hpp"
#import "Brain.hpp"
#import "Physics.hpp"

@implementation Reducer : NSObject

// we want to take a scene and produce the next name
// of course, we are going to mutate the scene, bc we're not sickos

- (id) initWithScene:(Scene *)scene {
    Reducer *reducer = [[Reducer alloc] init];
    
    reducer.scene = scene;
    reducer.brain = new Brain(scene);
    reducer.physics = new Physics(scene);
    
    return reducer;
}

- (matrix_float4x4) tick:(size_t)objectIndex {
 
    // for now would actually take other systems into account
    return _scene->transforms[objectIndex];
//    return matrix_identity_float4x4;
}

@end

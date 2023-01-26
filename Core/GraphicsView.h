//
//  GraphicsWindow.h
//  ToyRenderer
//
//  Created by Jordan Hall on 1/17/23.
//

#ifndef GraphicsView_h
#define GraphicsView_h

#import <MetalKit/MetalKit.h>
#import "MTKDelegationWrapper.h"

@interface GraphicsView: MTKView
- (BOOL)acceptsFirstResponder;
- (void)keyDown:(NSEvent*)event;
@property (weak) MTKDelegationWrapper* wrapper;
@end

#endif /* GraphicsView_h */

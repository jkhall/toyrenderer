//
//  GraphicsWindow.m
//  MetaliOS
//
//  Created by Jordan Hall on 1/17/23.
//

#import <Foundation/Foundation.h>
#import "GraphicsView.h"
#if defined(TARGETMACOS)
#import <Cocoa/Cocoa.h>
#endif

#import "BrickBreaker.hpp"

@implementation GraphicsView

- (BOOL)acceptsFirstResponder {
    return YES;
}

// probably going to have to take a reference to the wrapper...?
- (void)keyDown:(NSEvent *)event {
    NSLog(@"Keydown is firing");
    assert(_wrapper != nil);
    switch(event.keyCode) {
        case 0x7B:
            NSLog(@"Left arrow pressed");
            [_wrapper updateBrickBreakerFrameInput:LeftArrow];
            break;
        case 0x7C:
            NSLog(@"right arrow pressed");
            [_wrapper updateBrickBreakerFrameInput:RightArrow];
            break;
    }
}

@end

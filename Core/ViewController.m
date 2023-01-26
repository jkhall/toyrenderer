//
//  ViewController.m
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 6/26/22.
//

#import "ViewController.h"
#import <MetalKit/MetalKit.h>
#import "MTKDelegationWrapper.h"
#import "GraphicsView.h"

@implementation ViewController
{
//    MTKView* _view;
    GraphicsView* _view;
    MTKDelegationWrapper* _wrapper;
}

#if defined(TARGET_MACOS)
- (void)rightMouseDragged:(NSEvent *)event {
    NSLog(@"Right mouse dragged");
    [_wrapper rightMouseDragged:event];
}

- (void)mouseDown:(NSEvent *)event {
    NSLog(@"mouse clicked");
}

//- (void)keyDown:(NSEvent *)event {
//    NSLog(@"key down in viewcontroller");
////    [_wrapper keyDown:event];
//}

//- (BOOL)acceptsFirstResponder {
//    return YES;
//}
#endif


- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _view = (GraphicsView*)self.view;
    
//    _view.enableSetNeedsDisplay = YES;
    
    _view.device = MTLCreateSystemDefaultDevice();
    
//    _wrapper = [[MTKDelegationWrapper alloc] initWithView:_view];
    
    _wrapper = [[MTKDelegationWrapper alloc] initWithView2:_view];
    
    [_wrapper mtkView:_view drawableSizeWillChange:_view.drawableSize];
    
    _view.delegate = _wrapper;
    _view.wrapper = _wrapper;
    
//    BOOL res = [_view.window makeFirstResponder:_view];
//    BOOL areEqual = _view.window.firstResponder == _view;
    // start off paused
    // should I pause the view while reading from the file?
//    _view.paused = YES;
}

- (IBAction) takePicture:(id)sender {
    [_wrapper respondToTakePicture];
}


@end

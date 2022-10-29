//
//  ViewController.m
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 6/26/22.
//

#import "ViewController.h"
#import <MetalKit/MetalKit.h>
#import "MTKDelegationWrapper.h"

@interface ViewController ()
    
@end

@implementation ViewController
{
    MTKView* _view;
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
#endif


- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    _view = (MTKView*)self.view;
    
//    _view.enableSetNeedsDisplay = YES;
    
    _view.device = MTLCreateSystemDefaultDevice();
    
    _wrapper = [[MTKDelegationWrapper alloc] initWithView:_view];
    
    [_wrapper mtkView:_view drawableSizeWillChange:_view.drawableSize];
    
    _view.delegate = _wrapper;
   
    // start off paused
    // should I pause the view while reading from the file?
//    _view.paused = YES;
    
}

- (IBAction) takePicture:(id)sender {
    [_wrapper respondToTakePicture];
}


@end

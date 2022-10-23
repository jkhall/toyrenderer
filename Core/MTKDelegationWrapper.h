//
//  MTKDelegationWrapper.h
//  MetalCPPSetupiOS
//
//  Created by Jordan Hall on 6/26/22.
//

#ifndef MTKDelegationWrapper_h
#define MTKDelegationWrapper_h

#import <MetalKit/MetalKit.h>
//#import <Metal/Metal.h>

#import <CoreMotion/CoreMotion.h>
//#import "Scene.hpp"


@interface MTKDelegationWrapper: NSObject<MTKViewDelegate>
-(instancetype) initWithView:(MTKView*)view;
-(void) generateMipmapsForTexture:(id<MTLTexture>) texture onQueue:(id<MTLCommandQueue>)queue;
-(void) respondToTakePicture;
- (void)rightMouseDragged:(NSEvent*)event;

//#if defined(TARGET_IOS)
//-(id<MTLTexture>) loadMetalTexture:(NSString*)filename withCommandQueue:(id<MTLCommandQueue>)queue;
//-(uint8_t *)dataFromImage:(UIImage *)image;
//@property CMMotionManager* motion;
//#endif

//// generic properties
//@property (nonatomic, readwrite, assign) Renderer* renderer;
//@property Reducer *reducer;
//@property CGFloat zoomReference;
//@property Scene *defaultScene; // this should really be a static scene or something
//@property Scene *scene;
//@property MTKView *view;
//@property NSButton *loadDefaultSceneButton;
//@property NSTextField *sceneUnloadedText;

@end

#endif /* MTKDelegationWrapper_h */

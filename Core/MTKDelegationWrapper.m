//
//  MTKDelegationWrapper.m
//  MetalCPPSetupiOS
//
//  Created by Jordan Hall on 6/26/22.
//

//#import "MTKDelegationWrapper.h"
#import "Renderer.hpp"
#import "Reducer.h"
#import "Scene.hpp"
#import "OBJParser.hpp"
#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <MetalKit/MetalKit.h>
#import <Metal/Metal.h>
#import <vector>
#import <map>
#import "DelegateTest.h"
#import "TextTypes.h"

#if defined(TARGET_IOS)
#import <UIKit/UIKit.h>
#import <CoreMotion/CoreMotion.h>
#define PlatformFont UIFont
#elif defined(TARGET_MACOS)
#import <Cocoa/Cocoa.h>
#define PlatformFont NSFont
#endif

@interface MTKDelegationWrapper : NSObject<MTKViewDelegate>

-(instancetype) initWithView:(MTKView*)view;
-(void) generateMipmapsForTexture:(id<MTLTexture>) texture onQueue:(id<MTLCommandQueue>)queue;
-(void) respondToTakePicture;
-(void) rightMouseDragged:(NSEvent*)event;
-(uint8_t *) createTextBitmap;
-(float *) createSignedDistanceField:(uint8_t*)imageData width:(NSUInteger)width height:(NSUInteger)height;
-(float *) resampleDistanceField:(float *)distanceField width:(NSUInteger)width height:(NSUInteger)height scaleFactor:(NSUInteger)scaleFactor;
-(void) buildFontAtlas;
-(void) buildTextMesh;

@property (nonatomic, readwrite, assign) Renderer* renderer;
@property Reducer *reducer;
@property TextMeshProxy* textMeshProxy;
@property CGFloat zoomReference;
@property Scene *defaultScene; // this should really be a static scene or something
@property Scene *scene;
@property MTKView *view;
@property uint8_t *textImageData;
@property std::map<std::string, Scene*> sceneDictionary;
// text things
// MARK: TODO - Move this text shite
@property NSArray *glyphDescriptors;
@property CGFloat pointSize;
@property CTFontRef fontRef;
@property FontAtlas *atlas;
@property TextMesh *textMesh;
@property id<MTLTexture> fontTexture;

// end text shite
#if defined(TARGET_IOS)
@property CMMotionManager* motion;
@property CMAttitude* referenceAttitude;
#elif defined(TARGET_MACOS)
@property NSButton *loadDefaultSceneButton;
@property NSTextField *sceneUnloadedText;
#endif
@end

// Text Rendering Stuff
static NSString *const TRGlyphIndexKey = @"glyphIndex";
static NSString *const TRLeftTexCoordKey = @"leftTexCoord";
static NSString *const TRRightTexCoordKey = @"rightTexCoord";
static NSString *const TRTopTexCoordKey = @"topTexCoord";
static NSString *const TRBottomTexCoordKey = @"bottomTexCoord";
static NSString *const TRFontNameKey = @"fontName";
static NSString *const TRFontSizeKey = @"fontSize";
static NSString *const TRFontSpreadKey = @"spread";
static NSString *const TRTextureDataKey = @"textureData";
static NSString *const TRTextureWidthKey = @"textureWidth";
static NSString *const TRTextureHeightKey = @"textureHeight";
static NSString *const TRGlyphDescriptorsKey = @"glyphDescriptors";

static NSString *const FontName = @"Menlo Regular";
static float           FontAtlasSize = 2048;
static NSString *const TRSampleText = @"Tell me something good!";
static float TRFontDisplaySize = 72;

// glyph descriptor definition
// no bueno...
@implementation TRGlyphDescriptor

- (instancetype)initWithCoder:(NSCoder *)aDecoder
{
    if ((self = [super init]))
    {
        _glyphIndex = [aDecoder decodeIntForKey:TRGlyphIndexKey];
        _topLeft.x = [aDecoder decodeFloatForKey:TRLeftTexCoordKey];
        _topLeft.y = [aDecoder decodeFloatForKey:TRTopTexCoordKey];
        _bottomRight.x = [aDecoder decodeFloatForKey:TRRightTexCoordKey];
        _bottomRight.y = [aDecoder decodeFloatForKey:TRBottomTexCoordKey];
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)aCoder
{
    [aCoder encodeInt:self.glyphIndex forKey:TRGlyphIndexKey];
    [aCoder encodeFloat:self.topLeft.x forKey:TRLeftTexCoordKey];
    [aCoder encodeFloat:self.topLeft.y forKey:TRTopTexCoordKey];
    [aCoder encodeFloat:self.bottomRight.x forKey:TRRightTexCoordKey];
    [aCoder encodeFloat:self.bottomRight.y forKey:TRBottomTexCoordKey];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

@end


@implementation MTKDelegationWrapper

#if defined(TARGET_MACOS)
- (void)rightMouseDragged:(NSEvent*)event {
//    send the deltas to the renderer?
    // or deltas to axis angle rotations...?
    // around z -> a drag across full screen rotates 180 deg
    
    // this will give a camX and camZ
//    double angle = (_prevAngle + (M_PI * (event.deltaX / _view.frame.size.width))) % (2 * M_PI);
//    // update lookat -> // yeah?
//    _prevAngle = angle;
//    double radius = 10.0;
//    vector_float3 origin = {0, 0, 0};
    
    // let the renderer determine the angle from the delta, since prevAngle is private to the renderer
//    _renderer->updateLookat({(float)cos(angle), 0, (float)sin(angle)}, origin, {0, 1, 0}); // for now
    if(abs(event.deltaX) > abs(event.deltaY)){
        NSLog(@"Delta X: %f", event.deltaX);
    } else {
        NSLog(@"Delta Y: %f", event.deltaY);
    }
    _renderer->updateLookat(event.deltaX, event.deltaY); // just pass in deltas
}
#endif

#if defined(TARGET_IOS)
- (uint8_t *) dataFromImage:(UIImage *)image {
    CGImageRef imageRef = image.CGImage;
    
    const NSUInteger width = CGImageGetWidth(imageRef);
    const NSUInteger height = CGImageGetHeight(imageRef);
    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    uint8_t *rawData = (uint8_t *)calloc(height*width*4, sizeof(uint8_t));
    const NSUInteger bytesPerPixel = 4;
    const NSUInteger bytesPerRow = bytesPerPixel * width;
    const NSUInteger bitsPerComponent = 8;
    CGContextRef context = CGBitmapContextCreate(rawData,
                                                 width,
                                                 height,
                                                 bitsPerComponent,
                                                 bytesPerRow,
                                                 colorSpaceRef,
                                                 kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
    
    CGColorSpaceRelease(colorSpaceRef);
    
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1, -1);
    
    CGRect imageRect = CGRectMake(0, 0, width, height);
    CGContextDrawImage(context, imageRect, imageRef);
    
    CGContextRelease(context);
    
    return rawData;
}


- (id<MTLTexture>) loadMetalTexture:(NSString*) filename withCommandQueue:(id<MTLCommandQueue>)queue {
   // gonna go the core graphics route and see if that works as anticipated
    UIImage *image = [UIImage imageNamed:filename];
    
    CGSize imageSize = CGSizeMake(image.size.width  * image.scale, image.size.height * image.scale);
    const NSUInteger bytesPerPixel = 4;
    const NSUInteger bytesPerRow = bytesPerPixel * imageSize.width;
    uint8_t *data = [self dataFromImage:image];
    
    MTLTextureDescriptor *textureDescriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:imageSize.width height:imageSize.height mipmapped:NO];
    textureDescriptor.usage = MTLTextureUsageShaderRead;
    id<MTLTexture> texture = [[queue device] newTextureWithDescriptor:textureDescriptor];
    
    [texture setLabel:filename];
    
    MTLRegion region = MTLRegionMake2D(0, 0, imageSize.width, imageSize.height);
    [texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerRow];
    
    free(data);
 
    return texture;
}
#endif

-(void) buildFontAtlas {
    PlatformFont *font;
#if defined(TARGET_MACOS)
    font = [NSFont fontWithName:FontName size:32]; // don't know why it's this size from the example
#elif defined(TARGET_IOS)
    font = [UIFont fontWithName:FontName size:32];
#endif
    
    _atlas = [[FontAtlas alloc] initWithFont:font size:FontAtlasSize];
    
    MTLTextureDescriptor *textureDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm
                                                                                           width:FontAtlasSize
                                                                                          height:FontAtlasSize
                                                                                       mipmapped:NO];
    textureDesc.usage = MTLTextureUsageShaderRead;
    MTLRegion region = MTLRegionMake2D(0, 0, FontAtlasSize, FontAtlasSize);
    _fontTexture = [_view.device newTextureWithDescriptor:textureDesc];
    [_fontTexture setLabel:@"font texture"];
    [_fontTexture replaceRegion:region mipmapLevel:0 withBytes:_atlas.textureData.bytes bytesPerRow:FontAtlasSize];
}

-(void) buildTextMesh {
    // no idea about this...
    CGRect textRect = CGRectInset(_view.frame, 10, 10);
    
    _textMesh = [[TextMesh alloc] initWithString:TRSampleText inRect:textRect withFontAtlas:_atlas atSize:TRFontDisplaySize device:_view.device];
}

-(void) generateMipmapsForTexture:(id<MTLTexture>)texture onQueue:(id<MTLCommandQueue>)queue {
    id<MTLCommandBuffer> buffer = [queue commandBuffer];
    id<MTLBlitCommandEncoder> blitEncoder = [buffer blitCommandEncoder];
//    [blitEncoder generateMipmapsForTexture:texture];
    [blitEncoder endEncoding];
    [buffer commit];
    
    [buffer waitUntilCompleted];
}

- (instancetype)initWithView:(MTKView*) view {
    self = [super init];
    
    _view = view;
    
    NSLog(@"Window width: %f", _view.frame.size.width);
    NSLog(@"Window height: %f", _view.frame.size.height);
    
    // want to allow scene selection
    
    // default scene
    // how should we draw text in this context?
    
    _defaultScene = (Scene *)malloc(sizeof(Scene));
//    _defaultScene->rawModelNames[0] = "spot.obj";
//    _defaultScene->rawModelNames[0] = "plane";
    _defaultScene->rawModelNames[0] = "triangle";
    
//    _defaultScene->rawModelNames[1] = "plane"; // how might we put default geo in here
    _defaultScene->modelIndices = std::vector<size_t> {
            0,
    };
    
    vector_float3 vt = {0.0, 0.0, 0.0};
    vector_float3 planet = {0.0, 0.0, -5.0};
    
    _defaultScene->transforms = std::vector<matrix_float4x4> {
        matrix_identity_float4x4,
//        matrix_float4x4_translation(planet),
//        matrix_float4x4_translation(vt),
        // scale and translation

    //        matrix_float4x4_translation(vt)
    // matrix_identity_float4x4
    };

    _defaultScene->colors = std::vector<vector_float4> {
        (vector_float4){1.0, 0.0, 0.0, 1.0},
        (vector_float4){0.0, 1.0, 0.0, 1.0},
        (vector_float4){0.0, 1.0, 0.0, 1.0},
    };
    
    _sceneDictionary = {
        {"Default", _defaultScene},
    };
    
    if(_scene == nil) {
        _scene = _defaultScene;
    }

    // map modelFilenames and textureFilenames to actual resource paths
    for(int i = 0; i < sizeof(Renderer::modelFilenames)/sizeof(char *); i++){
        NSString *rawFilename = [NSString stringWithUTF8String:Renderer::modelFilenames[i]];

        Renderer::modelFilenameToResourcePath[[rawFilename UTF8String]] = [rawFilename UTF8String];

        NSArray *nameAndExtension = [rawFilename componentsSeparatedByString:@"."];
        NSString *filepath = [[NSBundle mainBundle] pathForResource:nameAndExtension[0] ofType:nameAndExtension[1]];

        Renderer::modelFilenameToResourcePath[std::string([rawFilename UTF8String])] = std::string([filepath UTF8String]);

    }

    for(int i = 0; i < sizeof(Renderer::textureFilenames)/sizeof(char *); i++){
        NSString *rawFilename = [NSString stringWithUTF8String:Renderer::textureFilenames[i]];
        NSArray *nameAndExtension = [rawFilename componentsSeparatedByString:@"."];
        NSString *filepath = [[NSBundle mainBundle] pathForResource:nameAndExtension[0] ofType:nameAndExtension[1]];

        Renderer::textureFilenameToResourcePath[[rawFilename UTF8String]] = [filepath UTF8String];
    }
    
    Renderer::dictsLoaded = true;

    // don't do this here i think or at least, not on this thread?
    // can I do this on another thread
//    [self loadScene:_scene];

    view.framebufferOnly = NO; // allows you to read the underlying texture in the drawable

    // get resources
//    NSMutableArray *resourcePaths = [[NSMutableArray alloc] init];
    std::vector<const char*> resourcePaths;
    std::vector<const char*> texturePathsCPP;

    // array of "top level" file names

    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"spot" ofType: @"obj"];
    resourcePaths.push_back(filePath.UTF8String);

//    // get texture filepaths
    NSArray* texturePaths = @[@"default-color"];
    for(NSString* texturePath in texturePaths) {
        NSString* path = [[NSBundle mainBundle] pathForResource:texturePath ofType: @"jpg"];
        texturePathsCPP.push_back(path.UTF8String);
    }


    // init renderer
    view.depthStencilPixelFormat = MTLPixelFormatDepth32Float;
    [view setClearDepth:1.0f];

//    // MARK: - Put extract this into function that can be called on a scene
//    // How do we do this when, we cant know what the scene is, until we consist the renderer?
//    // Doesn't the renderer have to reach out to this sort of code?
//    id<MTLCommandQueue> queue = [view.device newCommandQueue];
//
//    queue = [view.device newCommandQueue];
//
    self.renderer = new Renderer((__bridge_retained MTL::Device*)view.device, (NS::UInteger)view.colorPixelFormat, _scene, view.frame.size.width, view.frame.size.height);
//
#if defined(TARGET_OS)
    // motion
    self.motion = [[CMMotionManager alloc] init];

    [self.motion startDeviceMotionUpdatesUsingReferenceFrame:CMAttitudeReferenceFrameXArbitraryZVertical];

    // I do not think that I should automatically just use this frame
    self.referenceAttitude = self.motion.deviceMotion.attitude;

    // gesture recognition stuff
    UIPinchGestureRecognizer* pinchRecog = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];

    // double tap
    UITapGestureRecognizer* doubleTapRecog = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleDoubleTap:)];
    doubleTapRecog.numberOfTapsRequired = 2;

    [view addGestureRecognizer:pinchRecog];
    [view addGestureRecognizer:doubleTapRecog];
#endif

//    // reducer
//    // MARK: Todo - Create the scene here...
//    // how should we really do scene selection....
    self.reducer = [[Reducer alloc] initWithScene:self.defaultScene];
    
    // i want to have the font by this point
    
    // text stuff
    // possibly shouldn't be doing this in the initializer
    
    NSString *rawFontName = @"Menlo Regular";
    
    [self createTextureData]; // should be off the UI thread

    // should not be doing this in multiple places
//    NSUInteger boundsWidth = 4096;
//    NSUInteger boundsHeight = 4096;
//    NSUInteger textureSize = 4096;
//    NSUInteger fontAtlasSize = 4096;
//
//    CGRect rect = CGRectMake(0, 0, boundsWidth, boundsHeight);
//
//    NSInteger scaleFactor = fontAtlasSize / textureSize;
    
//    float spread = [self estimateGlyphWidth:(__bridge NSFont*)_fontRef];
//    
//    uint8_t* imageData = [self createTextBitmap];
//    float* sdf = [self createSignedDistanceField:imageData width:4096 height:4096];
//    float *scaledField = [self resampleDistanceField:sdf width:boundsWidth height:boundsHeight scaleFactor:scaleFactor];
//    
//    free(sdf);
//    
//    uint8_t* qImageData = [self quantizeDistanceField:scaledField width:4096 height:4096 normalizationFactor:spread];
//    
//    free(scaledField);
//    
//    NSInteger texByteCount = textureSize * textureSize;
//    
//    _atlas = [[FontAtlas alloc] init];
//    _atlas.textureData = [NSData dataWithBytesNoCopy:qImageData length:texByteCount freeWhenDone:YES];
//
    return self;
}

// for text...
-(void) createTextureData {
    [self buildFontAtlas];
    [self buildTextMesh];
    
    _textMeshProxy = (TextMeshProxy *)malloc(sizeof(TextMeshProxy));
    *_textMeshProxy = TextMeshProxy { .vertices = (__bridge MTL::Buffer*)_textMesh.textVertexBuffer, .indices = (__bridge MTL::Buffer*)_textMesh.textIndexBuffer, .texture = (__bridge MTL::Texture*)_fontTexture};
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    MTLRenderPassDescriptor* rpd = view.currentRenderPassDescriptor;
    
//    NSLog(@"running draw func"); <- seems to be running
    
    // yeah...
    // parse file
//    if(_scene->vertexOffsets.size() <= 0) return;

    
#if defined(TARGET_IOS)
    // we're gonna do this sorta thing
    // maybe what we really want is something more processed using this
    // getting processed device motion data page on apple developer
    // MARK: Todo - cmmotion should actually be put into it's own system...
    CMAttitude *attitude = self.motion.deviceMotion.attitude;
    if(attitude != nullptr) {
        self.renderer->setEuler(attitude.yaw, attitude.pitch, attitude.roll);
    }
#endif
    // MARK: Run updates here
    // physics
    // other kinds of updates
    
    _renderer->updateLookat(0.01, 0.0);
    
    if(!_scene->isLoaded) {
//        NSLog(@"scene not loaded");
        // the scene has already been passed to the renderer
        if(view.subviews.count == 0) {
            NSRect rect = NSMakeRect((_view.frame.size.width / 2) - 200, _view.frame.size.height / 2, 200, 20);
            _sceneUnloadedText = [[NSTextField alloc] initWithFrame:rect];
            _sceneUnloadedText.stringValue = @"Scene not loaded";
            [view addSubview:_sceneUnloadedText];

            NSRect btnFrame = NSMakeRect(_view.frame.size.width / 2, (_view.frame.size.height / 2) - 50, 200, 20);
            _loadDefaultSceneButton = [NSButton buttonWithTitle:@"Load Scene" target:self action:@selector(handleDefaultLoadScene:)];
            _loadDefaultSceneButton.frame = btnFrame;
            [view addSubview:_loadDefaultSceneButton];
        }
        return;
    }
    
    if(view.subviews.count == 2) {
//        [view.subviews[0] removeFromSuperview]; // yeah?
//        [view.subviews[1] removeFromSuperview];
        
        [_sceneUnloadedText removeFromSuperview];
        [_loadDefaultSceneButton removeFromSuperview];
    }
    
    _renderer->preDraw((__bridge MTL::RenderPassDescriptor*)rpd, (__bridge CA::MetalDrawable*)view.currentDrawable);
//    _renderer->startDraw((__bridge MTL::RenderPassDescriptor*)rpd, (__bridge CA::MetalDrawable*)view.currentDrawable);
//    for(int i = 0; i < _scene->modelIndices.size(); i++){
//        // the renderer has a _scene reference. why not just pass in the object index
//        // this is probably slow as shit because ptr following, etc...
//        // you do not want to load an unloaded scene
//        // would like for the renderer to fall back to something
//        _renderer->addDrawCommands(_scene->vertexOffsets[_scene->modelIndices[i]],
//                                   _scene->indexOffsets[_scene->modelIndices[i]],
//                                   _scene->indexOffsets[_scene->modelIndices[i] + 1] - _scene->indexOffsets[_scene->modelIndices[i]],
//                                   nullptr,
//                                   _scene->colors[i],
//                                   [_reducer tick:i]);
//    }
    
    // text
    _renderer->startDrawText((__bridge MTL::RenderPassDescriptor*)rpd, (__bridge CA::MetalDrawable*)view.currentDrawable);
    _renderer->addDrawCommandsText(_textMeshProxy); // no bridge call?
//    _renderer->endDrawText();
    
    
    _renderer->endDraw(); // drawable can only be presented once
    // should hand the transform off to the draw call
    // that said it should really be a set of transforms for all the objects
    
    // old draw function
    /*
    self.renderer->draw((__bridge MTL::RenderPassDescriptor*)rpd
                   ,(__bridge CA::MetalDrawable*) view.currentDrawable,
                        view.preferredFramesPerSecond);
     */
}

// what an actually insane bug
- (void) loadScene:(Scene *)scene {
    
    dispatch_queue_t q = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0);
    
    dispatch_async(q, ^{
        LoadModelDataFromScene(scene, (__bridge MTL::Device*)_view.device);
        scene->isLoaded = true;        
    });
    
    
    // MARK: Textures... load them into the scene
    
    // gonna want to also check if the loading failed right?
    
}


- (void) respondToTakePicture {
    NSLog(@"responding to take picture");
    self.renderer->takePicture();
}

// this would update some measure of the aspect ratio, which would change the perspective matrix calc
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    // not sure what to do here yet, with setting the viewport dimensions
    NSLog(@"setting viewport dimensions");
    self.renderer->setViewportSize(size.width, size.height);
}

- (IBAction) handleDefaultLoadScene:(NSObject *) sender {
    [self loadScene:_scene];
}

#if defined(TARGET_IOS)
// the scaling isn't very smooth
- (IBAction) handlePinch:(UIPinchGestureRecognizer *)recog {
    // I want to change the Z value of the objects in the scene based on the z value
//    self.renderer->setZoom(recog.scale); // this is a cgfloat will that be an issue???
    if(recog.state == UIGestureRecognizerStateBegan) {
        _zoomReference = recog.scale;
        return;
    } else {
        float zoomScale = 1.0f;
        float pinchScale = recog.scale < _zoomReference ? -recog.scale : recog.scale;
        _zoomReference = 1.0f;
        self.renderer->setZoom(pinchScale * zoomScale);
    }
}


- (IBAction) handleDoubleTap:(UITapGestureRecognizer *)recog {
    self.renderer->togglePicture();
}

#endif


@end

//
//  TextureLoader.h
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 7/25/22.
//

#ifndef TextureLoader_h
#define TextureLoader_h
#include <MetalKit/MetalKit.h>

@interface TextureLoader : NSObject
+(id<MTLTexture>)loadTextureUsingMetalKit: (NSURL *) url device: (id<MTLDevice>) device;
@end

#endif /* TextureLoader_h */

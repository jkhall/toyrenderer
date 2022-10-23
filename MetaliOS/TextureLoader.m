//
//  TextureLoader.m
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 7/25/22.
//

#import <Foundation/Foundation.h>
#import "TextureLoader.h"

@implementation TextureLoader

/*
 Why not pass in ptrs to images and MTL::Textures that are manipulated in place on the obj-c side?
 */

+(id<MTLTexture>)loadTextureUsingMetalKit: (NSURL *) url device: (id<MTLDevice>) device {
    MTKTextureLoader *loader = [[MTKTextureLoader alloc] initWithDevice: device];
    id<MTLTexture> texture = [loader newTextureWithContentsOfURL:url options:nil error:nil];
    
    if(!texture) {
        NSLog(@"Failed to create texture from: %@", url.absoluteString);
        return nil;
    }
    
    return texture;
};

@end

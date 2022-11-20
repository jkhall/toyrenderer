//
//  TextRenderer.m
//  MetalMac2
//
//  Created by Jordan Hall on 10/30/22.
//

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#if defined(TARGET_MACOS)
#define IndexType NSUInteger
#else
#endif

@interface TextRenderer : NSObject
@property NSFont* font;

@end

@implementation TextRenderer
// we'll just pass the font that we
- (id)init {
    TextRenderer* tr = [TextRenderer alloc];
    
    return tr;
}

@end

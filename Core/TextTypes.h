//
//  TextTypes.h
//  ToyRenderer
//
//  Created by Jordan Hall on 11/7/22.
//

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import <Metal/Metal.h>

#if defined(TARGET_MACOS)
#import <Cocoa/Cocoa.h>
#define PlatformFont NSFont
#elif defined(TARGET_IOS)
#define PlatformFont UIFont
#endif

#ifndef TextTypes_h
#define TextTypes_h

typedef void (^TRGlyphPositionEnumerationBlock)(CGGlyph glyph,
                                                NSInteger glyphIndex,
                                                CGRect glyphBounds);


typedef void (^GlyphEnumerationBlock) (CGGlyph glyph, NSInteger glyphIndex, CGRect glyphBounds);

@interface FontAtlas : NSObject <NSSecureCoding>
// methods
@property NSData *textureData;
@property NSFont *font; // what's the whole parentFont thing?
@property CGFloat pointSize;
@property CGFloat spread;
@property NSInteger textureSize;
@property NSArray *glyphDescriptors;
-(instancetype) initWithFont:(PlatformFont*)font size:(float)size;
-(CGFloat) pointSizeForFont:(PlatformFont*)font withBounds:(CGRect)rect;
-(CGSize) averageGlyphSize:(NSFont*)font;
-(CGFloat) estimateGlyphWidth:(NSFont*)font;
-(bool) font:(PlatformFont*)font withPointSize:(CGFloat)pointSize;
-(void) createTextureData;
-(uint8_t*) createTextBitmap;
-(float*) createSignedDistanceField:(const uint8_t*)imageData;
-(uint8_t*) quantizeDistanceField:(float *)distanceField;
-(float *) resampleDistanceField:(float *)distanceField;
@end

@interface TRGlyphDescriptor : NSObject <NSSecureCoding>
@property (nonatomic, assign) CGGlyph glyphIndex;
@property (nonatomic, assign) CGPoint topLeft;
@property (nonatomic, assign) CGPoint bottomRight;
@end

@interface TextMesh : NSObject
@property (nonatomic, readonly) NSData *textureData;
@property (nonatomic, readonly) id<MTLBuffer> textVertexBuffer;
@property (nonatomic, readonly) id<MTLBuffer> textIndexBuffer;
-(instancetype) initWithString:(NSString*)str inRect:(CGRect)textRect withFontAtlas:(FontAtlas*)fontAtlas atSize:(CGFloat)size device:(id<MTLDevice>)device;
-(void) buildMesh:(NSString*)string inRect:(CGRect)rect withFont:(FontAtlas *)atlas atSize:(CGFloat)fontSize device:(id<MTLDevice>)device;
-(void) enumerateGlyphsInFrame:(CTFrameRef)frame block:(TRGlyphPositionEnumerationBlock)block;
@end

#endif /* TextTypes_h */

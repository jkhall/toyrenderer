//
//  TextMesh.m
//  MetaliOS
//
//  Created by Jordan Hall on 11/12/22.
//

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>

#if defined(TARGET_MACOS)
#import <Cocoa/Cocoa.h>
#endif

#import <Metal/Metal.h>
#import "TextTypes.h"
#import "AAPLShaderTypes.h"

@implementation TextMesh
-(instancetype) initWithString:(NSString *)str inRect:(CGRect)textRect withFontAtlas:(FontAtlas *)fontAtlas atSize:(CGFloat)size device:(id<MTLDevice>)device {
    if((self = [super init]))
    {
        [self buildMesh:str inRect:textRect withFont:fontAtlas atSize:size device:device];
    }
    
    return self;
}

-(void) buildMesh:(NSString *)string inRect:(CGRect)rect withFont:(FontAtlas *)atlas atSize:(CGFloat)fontSize device:(id<MTLDevice>)device {
    NSFont* font = [atlas.font fontWithSize:fontSize];
    NSDictionary *attr = @{NSFontAttributeName: font};
    NSAttributedString *attrString = [[NSAttributedString alloc] initWithString:string attributes:attr];
    NSInteger lng = attrString.length;
    CFRange strRange = CFRangeMake(0, attrString.length);
    CGPathRef rectPath = CGPathCreateWithRect(rect, NULL);
    CTFramesetterRef framesetterRef = CTFramesetterCreateWithAttributedString((__bridge CFAttributedStringRef) attrString);
    CTFrameRef frameRef = CTFramesetterCreateFrame(framesetterRef, strRange, rectPath, NULL);
    
    
    
    __block CFIndex frameGlyphCount = 0;
    NSArray *lines = (__bridge id)CTFrameGetLines(frameRef);
    
    [lines enumerateObjectsUsingBlock:^(id lineObject, NSUInteger lineIndex, BOOL *stop) {
        frameGlyphCount += CTLineGetGlyphCount((__bridge CTLineRef)lineObject);
    }];
    
    const NSUInteger vertexCount = frameGlyphCount * 4; // corners
    const NSUInteger indexCount = frameGlyphCount * 6; // two triangles
    MBEVertex *vertices = (MBEVertex*)malloc(vertexCount * sizeof(MBEVertex));
    MBEIndex *indices = (MBEIndex*)malloc(indexCount * sizeof(MBEIndex));
    
    __block MBEIndex v = 0, i = 0;
    [self enumerateGlyphsInFrame:frameRef block:^(CGGlyph glyph, NSInteger glyphIndex, CGRect glyphBounds) {
        if(glyph >= atlas.glyphDescriptors.count) {
            NSLog(@"Font atlas has no entry corresponding to glyph");
            return; // skip
        }
        
        TRGlyphDescriptor *glyphInfo = atlas.glyphDescriptors[glyph];
        float minX = CGRectGetMinX(glyphBounds);
        float maxX = CGRectGetMaxX(glyphBounds);
        float minY = CGRectGetMinY(glyphBounds);
        float maxY = CGRectGetMaxY(glyphBounds);
        
        float minS = glyphInfo.topLeft.x;
        float maxS = glyphInfo.bottomRight.x;
        float minT = glyphInfo.topLeft.y;
        float maxT = glyphInfo.bottomRight.y;
        
        vertices[v++] = (MBEVertex){ { minX, maxY, 0, 1 }, {0,0,0,0}, {0,0,0,0}, { minS, maxT } };
        vertices[v++] = (MBEVertex){ { minX, minY, 0, 1 }, {0,0,0,0}, {0,0,0,0}, { minS, minT } };
        vertices[v++] = (MBEVertex){ { maxX, minY, 0, 1 }, {0,0,0,0}, {0,0,0,0}, { maxS, minT } };
        vertices[v++] = (MBEVertex){ { maxX, maxY, 0, 1 }, {0,0,0,0}, {0,0,0,0}, { maxS, maxT } };
        indices[i++] = glyphIndex * 4;
        indices[i++] = glyphIndex * 4 + 1;
        indices[i++] = glyphIndex * 4 + 2;
        indices[i++] = glyphIndex * 4 + 2;
        indices[i++] = glyphIndex * 4 + 3;
        indices[i++] = glyphIndex * 4;
    }];
    
    _textVertexBuffer = [device newBufferWithBytes:vertices
                                            length:vertexCount * sizeof(MBEVertex)
                                           options:MTLResourceOptionCPUCacheModeDefault];
    [_textVertexBuffer setLabel:@"textVertexBuffer"];
    
    _textIndexBuffer = [device newBufferWithBytes:indices
                                           length:indexCount * sizeof(MBEIndex)
                                          options:MTLResourceCPUCacheModeDefaultCache];
    [_textIndexBuffer setLabel:@"textIndexBuffer"];
    
    free(indices);
    free(vertices);
    CFRelease(frameRef);
    CFRelease(framesetterRef);
    CFRelease(rectPath);
}

-(void) enumerateGlyphsInFrame:(CTFrameRef)frame block:(TRGlyphPositionEnumerationBlock)block {
    if(!block) return;
    
    CFRange entire = CFRangeMake(0, 0);
    CGPathRef framePath = CTFrameGetPath(frame);
    CGRect frameBoundingRect = CGPathGetPathBoundingBox(framePath);
    
    NSArray *lines = (__bridge id)CTFrameGetLines(frame);
    
    CGPoint *lineOriginBuffer = (CGPoint*)malloc(lines.count * sizeof(CGPoint));
    CTFrameGetLineOrigins(frame, entire, lineOriginBuffer);
    
    __block CFIndex glyphIndexInFrame = 0;
    
    //    CGContextRef ref = CGContextCreate
    NSImage *image = [[NSImage alloc] initWithSize:CGSizeMake(1, 1)];
    NSBitmapImageRep *imageRep = [[NSBitmapImageRep alloc] initWithCGImage:(__bridge CGImage*)image];
    NSGraphicsContext *context = [NSGraphicsContext graphicsContextWithBitmapImageRep:imageRep];
    
    [lines enumerateObjectsUsingBlock:^(id lineObject, NSUInteger lineIndex, BOOL *stop) {
        CTLineRef line = (__bridge CTLineRef)lineObject;
        CGPoint lineOrigin = lineOriginBuffer[lineIndex];
        
        NSArray *runs = (__bridge id)CTLineGetGlyphRuns(line);
        [runs enumerateObjectsUsingBlock:^(id runObject, NSUInteger rangeIndex, BOOL *stop) {
            CTRunRef run = (__bridge CTRunRef)runObject;
            CGContextRef ctx = (__bridge CGContextRef)context;
            
            NSInteger glyphCount = CTRunGetGlyphCount(run);
            
            CGGlyph *glyphBuffer = (CGGlyph *)malloc(glyphCount * sizeof(CGGlyph));
            CTRunGetGlyphs(run, entire, glyphBuffer);
            
            CGPoint *positionBuffer = (CGPoint *)malloc(glyphCount * sizeof(CGPoint));
            CTRunGetPositions(run, entire, positionBuffer);
            
            for (NSInteger glyphIndex = 0; glyphIndex < glyphCount; glyphIndex++) {
                CGGlyph glyph = glyphBuffer[glyphIndex];
                CGPoint glyphOrigin = positionBuffer[glyphIndex];
                CGRect glyphRect = CTRunGetImageBounds(run, ctx, CFRangeMake(glyphIndex, 1));
                CGFloat boundsTransX = frameBoundingRect.origin.x + lineOrigin.x;
                CGFloat boundsTransY = CGRectGetHeight(frameBoundingRect) + frameBoundingRect.origin.y - lineOrigin.y + glyphOrigin.y;
                CGAffineTransform pathTransform = CGAffineTransformMake(1, 0, 0, -1, boundsTransX, boundsTransY);
                glyphRect = CGRectApplyAffineTransform(glyphRect, pathTransform);
                block(glyph, glyphIndexInFrame, glyphRect);
                
                glyphIndexInFrame++;
            }
            
            free(positionBuffer);
            free(glyphBuffer);
        }];
    }];
}
@end


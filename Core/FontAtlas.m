//
//  FontAtlas.m
//  MetaliOS
//
//  Created by Jordan Hall on 11/12/22.
//

#import <Foundation/Foundation.h>
#import <CoreFoundation/CoreFoundation.h>
#import <CoreText/CoreText.h>
#import "TextTypes.h"
#import <simd/simd.h>

#if defined(TARGET_MACOS)
#import <Cocoa/Cocoa.h>
#define PlatformFont NSFont
#elif defined(TARGET_IOS)
#define PlatformFont UIFont
#endif

@implementation FontAtlas

static NSString *const TRFontName = @"Menlo Regular";
static vector_float4 TextColor = {0.1, 0.1, 0.1, 1};
static float TRFontAtlasSize = 4096;


-(instancetype) initWithFont:(PlatformFont *)font size:(float)size {
    // when would this return false?
    if(self = [super init]) {
        _font = font;
        _pointSize = font.pointSize;
        _spread = [self estimateGlyphWidth:font] * 0.5;
        _glyphDescriptors = [NSMutableArray array];
        _textureSize = size;
        [self createTextureData];
    }
    
    return self;
}

-(CGSize) averageGlyphSize:(PlatformFont*)font {
    CGFloat cumulativeWidth = 0;
    CGFloat maxHeight = -1;
    NSUInteger numberOfGlyphs = [font numberOfGlyphs];
    
    
    for(CGGlyph glyph = 0; glyph < numberOfGlyphs; glyph++) {
        CGRect boundingRect;
//        CTFontGetBoundingRectsForGlyphs(font, kCTFontOrientationHorizontal, &glyph, &boundingRect, numberOfGlyphs);
        boundingRect = [font boundingRectForCGGlyph:glyph];
        cumulativeWidth += boundingRect.size.width;
        if(boundingRect.size.height > maxHeight) {
            maxHeight = boundingRect.size.height;
        }
    }
    
    CGFloat averageWidth = cumulativeWidth / numberOfGlyphs;
    
    return CGSizeMake(averageWidth, maxHeight);
}


-(CGFloat) estimateGlyphWidth:(PlatformFont*)font {
    CGFloat estimatedStrokeWidth = [@"!" sizeWithAttributes:@{NSFontAttributeName: font}].width;
    return ceilf(estimatedStrokeWidth);
}

-(bool)font:(PlatformFont*)font withPointSize:(CGFloat)pointSize shouldFit:(CGRect)rect {
    
    PlatformFont *trialFont = [PlatformFont fontWithName:font.fontName size:pointSize];
    CTFontRef trialCTFont = CTFontCreateWithName((__bridge CFStringRef)font.fontName, pointSize, NULL);
    CGSize averageGlyphSizeF = [self averageGlyphSize:trialFont];
    CGFloat marginWidth = [self estimateGlyphWidth:trialFont];
    NSUInteger gCount = [trialFont numberOfGlyphs];
    CGFloat fontArea = (averageGlyphSizeF.width + marginWidth) * (averageGlyphSizeF.height + marginWidth) * gCount;
    CGFloat rectArea = rect.size.width * rect.size.height;
    
    CFRelease(trialCTFont);
    return rectArea > fontArea;
}

-(CGFloat) pointSizeForFont:(PlatformFont*)font withBounds:(CGRect)rect {
    
    // what should be the default pointSize?
    CGFloat ps = font.pointSize;
    
    
    while([self font:font withPointSize:ps shouldFit:rect]) ++ps;
    while(![self font:font withPointSize:ps shouldFit:rect]) --ps;
    
    // return new font here?
    
    return ps;
}

-(uint8_t*) createTextBitmap {
    size_t width = 4096;
    size_t height = 4096;
    
    uint8_t *data = (uint8_t *)malloc(width*height);
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGBitmapInfo bitmapInfo = (kCGBitmapAlphaInfoMask & kCGImageAlphaNone);
    CGContextRef context = CGBitmapContextCreate(data,
                                              width,
                                              height,
                                              8,
                                              width,
                                              colorSpace,
                                              bitmapInfo);
    
    CGContextSetAllowsAntialiasing(context, false);
    CGContextTranslateCTM(context, 0, height);
    CGContextScaleCTM(context, 1, -1);
    // not sure why but we're going to flip the context?
    
    // fill with black
    CGRect rect =  CGRectMake(0, 0, width, height);
    CGContextSetRGBFillColor(context, 0, 0, 0, 1);
    CGContextFillRect(context, rect);
    
//    NSString * rawFontName = @"HoeflerText-Regular";
//    NSString *rawFontName = @"Menlo Regular";
    _pointSize = [self pointSizeForFont:_font withBounds:rect];
    CTFontRef fontRef = CTFontCreateWithName((__bridge CFStringRef)_font.displayName, _pointSize, NULL);
    _font = [NSFont fontWithName:_font.displayName size:_pointSize];
    
    NSUInteger glyphCount = [(__bridge NSFont*)fontRef numberOfGlyphs];

    CGFloat fontMaxAscent = [(__bridge NSFont*)fontRef ascender];
    CGFloat fontMinDescent = [(__bridge NSFont*)fontRef descender];
    CGFloat otherAscender = CTFontGetAscent(fontRef);
    CGFloat otherDecsender = CTFontGetDescent(fontRef);
    CGFloat glyphMargin = [self estimateGlyphWidth:(__bridge NSFont*)fontRef];
    
    CGPoint origin = CGPointMake(0, fontMaxAscent);
    CGFloat maxYForLine = -1;
    
    CGContextSetRGBFillColor(context, 1, 1, 1, 1);
    
    NSMutableArray *mutableGlyphs = (NSMutableArray*)_glyphDescriptors;
    [mutableGlyphs removeAllObjects];
    
    // MARK: Trying to get higher resolution texture
//    NSString *sampleStr = @"abcdefghijklmnopqrztuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!.,";
//    NSAttributedString *attrStr = [[NSAttributedString alloc] initWithString:sampleStr];
//    [attrStr setValue:_font.displayName forKey:@"font"]; // maybe?
//    CTLineRef lines = CTLineCreateWithAttributedString((__bridge CFMutableAttributedStringRef)attrStr);
//    CFArrayRef arr = CTLineGetGlyphRuns(lines);
    // ------------------------------------------------
    
    // i would have to call this inside of the enumeration from text mesh... I think
    // "this" meaning the content of the for loop only
    for(CGGlyph g = 0; g < glyphCount; g++) {
        // mark up the bitmap
        CGRect boundingRect;
        boundingRect = [(__bridge NSFont*)fontRef boundingRectForCGGlyph:g];
        
        // if too large in the x
        if((origin.x + boundingRect.origin.x) + glyphMargin > width) {
            origin.x = 0;
            origin.y = maxYForLine + glyphMargin + otherDecsender;
        }
        
        // update max y for line
        if(origin.y + CGRectGetMaxY(boundingRect) > maxYForLine) {
            maxYForLine = origin.y + CGRectGetMaxY(boundingRect);
        }
        
        CGFloat glyphX = origin.x - boundingRect.origin.x + (glyphMargin * 0.5); // this feels weird for me
        CGFloat glyphY = origin.y + (glyphMargin * 0.5);
        
        // i need more experience with coord graphics
        // -1 bc of the inversion that we do?
        CGAffineTransform glyphTransform = CGAffineTransformMake(1, 0, 0, -1, glyphX, glyphY);
        
        CGPathRef path = CTFontCreatePathForGlyph(fontRef, g, &glyphTransform);
        CGContextAddPath(context, path);
        CGContextFillPath(context);
        
        CGRect glyphPathBoundingRect = CGPathGetPathBoundingBox(path);
        
        if(CGRectEqualToRect(rect, CGRectNull)){
            glyphPathBoundingRect = CGRectZero; // rather than inf
        }
        
        // texture stuff, yo
        // descriptors yo
        // ignore for now, this is for making the mesh
        
        CGFloat texCoordLeft = glyphPathBoundingRect.origin.x / width;
        CGFloat texCoordRight = (glyphPathBoundingRect.origin.x + glyphPathBoundingRect.size.width) / width;
        CGFloat texCoordTop = glyphPathBoundingRect.origin.y / height;
        CGFloat texCoordBottom = (glyphPathBoundingRect.origin.y + glyphPathBoundingRect.size.height) / height;
        
        // save glyph descriptors for some things
        TRGlyphDescriptor *descrip = [[TRGlyphDescriptor alloc] init];
        descrip.glyphIndex = g;
        descrip.topLeft = CGPointMake(texCoordLeft, texCoordTop);
        descrip.bottomRight = CGPointMake(texCoordRight, texCoordBottom);
        
        // mutable glyphs
        [mutableGlyphs addObject:descrip];
        
        
        CGPathRelease(path);
        
//        origin.x += boundingRect.size.width + (glyphMargin);
        origin.x += CGRectGetWidth(boundingRect) + glyphMargin;
    }
    
    CGImageRef imageRef = CGBitmapContextCreateImage(context);
    NSImage *image = [[NSImage alloc] initWithCGImage:imageRef size:rect.size];
    image = nil;
    CGImageRelease(imageRef);
    
    return data;
}

-(uint8_t *) quantizeDistanceField:(float *)distanceField width:(NSUInteger)width height:(NSUInteger)height normalizationFactor:(float)factor {
    uint8_t *qData = (uint8_t *)malloc(width*height);
    
    for(int y = 0; y < height; ++y) {
        for(int x = 0; x < width; ++x) {
            float dist = distanceField[y * width + x];
            float clampDist = fmax(-factor, fmin(dist, factor));
            float scaledDist = clampDist / factor;
            
            uint8_t val = ((scaledDist + 1) / 2) * UINT8_MAX;
            qData[(y * width) + x] = val;
        }
    }
    
    return qData;
}


-(float*) createSignedDistanceField:(const uint8_t*)imageData width:(NSInteger)width height:(NSInteger)height {
    if(width == 0 || height == 0 || imageData == NULL) {
        return NULL;
    }
    
    // custom struct
    typedef struct { unsigned short x, y; } intpoint_t;
    
    // distance field
    float* distanceField = (float*)malloc(sizeof(float)*width*height);
    intpoint_t* boundaryPointMap = (intpoint_t*)malloc(sizeof(intpoint_t)*width*height);
    
    const float maxDist = hypot(width, height);
    const float distUnit = 1;
    const float distDiag = sqrt(2);
    
#define image(_x, _y) (imageData[(_y) * width + (_x)] > 0x7f)
#define distance(_x, _y) distanceField[(_y) * width + (_x)]
#define nearestpt(_x, _y) boundaryPointMap[(_y) * width + (_x)]
    
    // initialize distance field to inf everywhere
    for(long y = 0; y < height; y++) {
        for(long x = 0; x < width; x++) {
            distance(x, y) = maxDist;
            nearestpt(x, y) = (intpoint_t) {0, 0};
        }
    }
    
    // loop over image find boundary points
    for(long y = 1; y < height - 1; ++y) {
        for(long x = 1; x < width - 1; ++x) {
            // if boundary point
            // pt is different value in any of immediate cardinal dirs
            bool inside = image(x, y);
            if(image(x - 1, y) != inside ||
               image(x + 1, y) != inside ||
               image(x, y - 1) != inside ||
               image(x, y + 1) != inside) {
                distance(x, y) = 0; // distance to edge
                nearestpt(x, y) = (intpoint_t){static_cast<unsigned short>(x), static_cast<unsigned short>(y)};
            }
        }
    }
    
//    float maxDistance = -1;
//    float minDistance = 1;
//    intpoint_t maxDistancePt {0, 0};
//    intpoint_t minDistancePt {0, 0};
    
    // forward pass
    for(long y = 1; y < height - 2; ++y) {
        for(long x = 1; x < width - 2; ++x) {
            if(distance(x - 1, y - 1) + distDiag < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x - 1, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x, y - 1) + distUnit < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x + 1, y - 1) + distDiag < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x + 1, y - 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x - 1, y) + distUnit < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x - 1, y);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            
//            if(distance(x, y) > maxDistance){
//                maxDistance = distance(x, y);
//                maxDistancePt = {x, y};
//            }
//            if(distance(x, y) < minDistance){
//                minDistance = distance(x, y);
//                minDistancePt = {x, y};
//            }
        }
    }
    
    // backward pass
    for(long y = height - 2; y >= 1; --y) {
        for(long x = width - 2; x >= 1; --x) {
            if(distance(x + 1, y) + distUnit < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x + 1, y);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x - 1, y + 1) + distDiag < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x - 1, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x, y + 1) + distUnit < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            if(distance(x + 1, y + 1) + distDiag < distance(x, y)) {
                nearestpt(x, y) = nearestpt(x + 1, y + 1);
                distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
            }
            
//            if(distance(x, y) > maxDistance){
//                maxDistance = distance(x, y);
//                maxDistancePt = {x, y};
//            }
//            if(distance(x, y) < minDistance){
//                minDistance = distance(x, y);
//                minDistancePt = {x, y};
//            }
        }
    }
    
    // negation pass
    // "outside" distances are turnd to negative
    for(long y = 0; y < height; ++y) {
        for(long x = 0; x < width; ++x) {
            if(!image(x, y)) {
                distance(x, y) = -distance(x, y);
//                if(x == maxDistancePt.x && y == maxDistancePt.y) {
//                    maxDistance = distance(x, y);
//                }
//                if(x == minDistancePt.x && y == minDistancePt.y) {
//                    minDistance = distance(x, y);
//                }
            }
        }
    }
    
    free(boundaryPointMap);
    
    return distanceField;
}

-(float *) resampleDistanceField:(float *)distanceField width:(NSUInteger)width height:(NSUInteger) height scaleFactor:(NSUInteger) scaleFactor {
    NSAssert(width % scaleFactor == 0 && height % scaleFactor == 0, @"Dimensions aren't evenly divisible by scale factor");
    
    NSUInteger scaledWidth = width / scaleFactor;
    NSUInteger scaledHeight = height / scaleFactor;
    float *outData = (float *)malloc(scaledWidth * scaledHeight * sizeof(float));
    
    for(int y = 0; y < height; y += scaleFactor) {
        for(int x = 0; x < width; x += scaleFactor) {
            float accum = 0;
            for(int ky = 0; ky < scaleFactor; ky++) {
                for(int kx = 0; kx < scaleFactor; kx++) {
                    accum += distanceField[(y + ky) * width + (x + kx)];
                }
            }
            accum = accum / (scaleFactor * scaleFactor);
            
            outData[(y / scaleFactor) * scaledWidth + (x / scaleFactor)] = accum;
        }
    }
    
    return outData;
}

-(void) createTextureData {
    uint8_t *bitmapData = [self createTextBitmap];
    NSInteger scaleFactor = TRFontAtlasSize / _textureSize;
    
    float *distanceField = [self createSignedDistanceField:bitmapData width:TRFontAtlasSize height:TRFontAtlasSize];
    float *scaledField = [self resampleDistanceField:distanceField width:TRFontAtlasSize height:TRFontAtlasSize scaleFactor:scaleFactor];
    
    free(distanceField);
    
    CGFloat spread = [self estimateGlyphWidth:_font] * 0.5;
    
    uint8_t *texture = [self quantizeDistanceField:scaledField width:_textureSize height:_textureSize normalizationFactor:spread];
    
//    CGImageRef imageRef = CGBitmapContextCreateImage(context);
//    NSImage *image = [[NSImage alloc] initWithCGImage:imageRef size:rect.size];
//    image = nil;
//    CGImageRelease(imageRef);
    
    
    free(scaledField);
    
    NSInteger textureByteCount = _textureSize * _textureSize;
    CGRect textRect = CGRectMake(0, 0, _textureSize, _textureSize);
    
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
    CGBitmapInfo bitmapInfo = (kCGBitmapAlphaInfoMask & kCGImageAlphaNone);
    CGContextRef context = CGBitmapContextCreate(texture,
                                              _textureSize,
                                              _textureSize,
                                              8,
                                              _textureSize,
                                              colorSpace,
                                              bitmapInfo);
    CGImageRef imageRef = CGBitmapContextCreateImage(context);
    
    // how to replace rect here
    // i guess this looks ok...
    NSImage *image = [[NSImage alloc] initWithCGImage:imageRef size:textRect.size];
    image = nil;
    CGImageRelease(imageRef);
    
    
    _textureData = [NSData dataWithBytesNoCopy:texture length:textureByteCount freeWhenDone:YES];
}


@end

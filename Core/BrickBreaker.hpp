//
//  BrickBreaker.hpp
//  MetaliOS
//
//  Created by Jordan Hall on 12/5/22.
//

#ifndef BrickBreaker_hpp
#define BrickBreaker_hpp

#include <stdio.h>
#include "Scene.hpp"
#include "Renderer.hpp"
#include "AAPLShaderTypes.h"
#include <Metal/Metal.hpp>
#include <simd/simd.h>
// maybe put the image in here?
// and the 2D coordinates?
// we actually just want all these vertices in a large array

// these would be for the purposes of simulation and constructing frame data
typedef struct {
    vector_int2 pos;
    float scale;
    float rot;
} Ball;

typedef struct {
    vector_float2 pos;
    float scale;
    float rot;
    int health;
} Brick;

// have a power up type
typedef struct {
    vector_int2 pos;
    float scale;
    float rot;
} PowerUp;

typedef struct {
    vector_int2 pos;
    float scale;
    float rot;
    vector_float4 color; // maybe
} Board;

enum EntityType {
    BallType = 0,
    BrickType = 1,
    BoardType = 2,
    PaddleType = 3,
};

// powers of 2 here
enum Input {
    Nothing = 0,
    LeftArrow = 1,
    RightArrow = 2,
};

const vector_float4 BOARD_COLOR = {0.145, 0.145, 0.145, 1};
const vector_float4 BRICK_COLOR = {1,     1,     1,     1};
// -------------

// brickbreaker uniforms

// --------------

class BrickBreaker {
public:
    BrickBreaker(MTL::Device* device, float viewWidth, float viewHeight);
    ~BrickBreaker();
    
    void CopyInstanceData(int frame);
    void Draw(Renderer *renderer);
    void Update(double timeStep, uint16_t frameInput = 0);
    void UpdateSizes(float viewWidth, float viewHeight);
    void UpdateVertices();
    void UpdateBaseTransforms();
    bool IsLoaded();
    
    Scene *scene;
    MTL::Buffer *brickInstances[3];
    MTL::Buffer *ballInstances[3];
    
    // I want the board to be 1 : 2 -> width: height
    // this should inform the min size of the window
    int boardWidth;
    int boardHeight;
    int viewWidth;
    int viewHeight;
    int fractionalBrickWidth;
    int paddleUnitHeight;
    int paddleUnitWidth; // meant to be multiplied by our "unit"
    int boardLeftX, boardRightX, boardTopY, boardBottomY;
        
    uint16_t frameInput;
    vector_float2 paddleTarget = {0, 0};
    float lastPaddleXUpdate;
    vector_float2 ballTarget = {0, 0};
    vector_float2 ballMotion = {0, 0};
    
    float brickHeight;
    float brickWidth;
    uint32_t brickCollisions = 0;
    
    int widthScale;
    int heightScale;
    int renderableCount;
    float unitSize;
    float speed = 1000.0; // i hate this
    
    const vector_float4 clearColor = {0.2, 0.2, 0.2, 1.0};
    
    // statics
    static const char* ballTextureFilename; // can you just implicitly render a ball here? like something based on the radius?
    static const char* brickTextureFilename;
    static const char* powerUpTextFilenames[5];
    static const int ballVertexCount;
    static const int brickVertexCount;
    static const int powerUpVertexCount;
private:
    Ball *ball;
    Brick *bricks;
    PowerUp *powerUps;
    Board *board;
    Renderable *renderables;
    
    Transform *transforms;
    
    size_t brickcount;
    size_t boardcount;
    size_t ballcount;
    size_t paddlecount;
    size_t entitycount;
    
    size_t paddleIndex;
    size_t ballIndex;
};



#endif /* BrickBreaker_hpp */

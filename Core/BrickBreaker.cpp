//
//  BrickBreaker.cpp
//  MetaliOS
//
//  Created by Jordan Hall on 12/5/22.
//

#include "BrickBreaker.hpp"
#include "AAPLShaderTypes.h"
#include "Renderer.hpp"
#include <stdio.h>
#include <iostream>
#include "Utils.hpp"

const int BrickBreaker::ballVertexCount = 4;
const int BrickBreaker::brickVertexCount = 4;
const int BrickBreaker::powerUpVertexCount = 4;

//-------------------------------------------
// Components
//-------------------------------------------

// a renderable should have a model reference and a transform while the offsets can be looked up elsewhere...
BrickBreaker::BrickBreaker(MTL::Device *device, float viewW, float viewH) {
    brickcount = 20;
    ballcount = 1;
    boardcount = 1;
    paddlecount = 1;
    boardWidth = 500; // should be dependent on the view...
    boardHeight = 675;
    renderableCount = 4; // board, brick, ball, paddle
    unitSize = 10;
    entitycount = ballcount + boardcount + brickcount + paddlecount;
    viewWidth = viewW;
    viewHeight = viewH;
    paddleUnitWidth = 6;
    paddleUnitHeight = 1;
    
    paddleIndex = brickcount + boardcount + ballcount;
    ballIndex = 0;
    
    float halfViewWidth = viewWidth / 2;
    float halfViewHeight = viewHeight / 2;
    float halfBoardWidth = boardWidth / 2;
    float halfBoardHeight = boardHeight / 2;			
    fractionalBrickWidth = 5; // percentage
    brickHeight = 1; // units
    float ballWidth = 25;
    float ballHeight = 25;
    float paddleWidth = 2;
    float paddleHeight = 1;
    
    // You have a Thing -> Associated with a Mesh (vertices, indices) -> And some kind of behavior (physically driven, or otherwise)
    // it would be nice to just load things into some single place here, the scene, but I don't think it's setup quite right at the moment
    // maybe ECS could give me some insight? Before looking into it, to better understand, I think I want to struggle and do it wrong first
    // do see the benefits and better be able to assess what I want to take and don't later on
    // I think it's possible that pos, rot would be in a particular component, passed to some system
    // pos then passed to the userinput system, etc?
    
    // i want to describe points directly in ndc, not project them to ndc, because the points aren't going to have anything to do
    // with the screen
    
    scene = (Scene*)malloc(sizeof(Scene));
    
    // what's the interface I actually want?
    // just files in a folder and then have the scene set up automatically
    // but then, what if you end up including things multiple times?
    // where / when do we load the scene?
    // we need a separate  fragment function since we're not sampling from a texture
    // need a better way to calculate "uniforms"
    
    
//    int vertexCount = ballVertexCount + (brickcount * brickVertexCount) + powerUpVertexCount;
//    int indexCount = (((ballVertexCount * ballcount) - 2) + ((brickcount * brickVertexCount) - 2) + (powerUpVertexCount - 2)) * 3;
//    MBEVertex *vertices = (MBEVertex*)malloc(sizeof(MBEVertex) * vertexCount);
//    MBEIndex *indices = (MBEIndex*)malloc(sizeof(MBEIndex) * indexCount);
    
//    int vertexCount = ballVertexCount + brickVertexCount + powerUpVertexCount;
//    int indexCount = ((ballVertexCount - 2) + (brickVertexCount - 2) + (powerUpVertexCount - 2)) * 3;
    
    int vertexCount = 16;
    int indexCount = 24;
    
//    MBEVertex *vertices = (MBEVertex*)malloc(sizeof(MBEVertex) * vertexCount);
//    MBEIndex *indices = (MBEIndex*)malloc(sizeof(MBEIndex) * indexCount);
    
    // gotta fix these
    MBEIndex indices[] = {
        // ball: 0 - 6
        // I think these are all the same, bc they're all quads...
        0, 1, 2,
        2, 3, 0,
        // brick: 6 - 12
        4, 5, 6,
        6, 7, 4,
        // screen: 12 - 18
        8, 9, 10,
        10, 11, 8,
        // paddle: 18 - 24
        12, 13, 14,
        14, 15, 12,
    };
    
    // to create anything in metal you need a reference to a device...
    scene->vertices = device->newBuffer(sizeof(MBEVertex) * vertexCount, MTL::ResourceOptionCPUCacheModeDefault);
    scene->indices = device->newBuffer(sizeof(MBEIndex) * indexCount, MTL::ResourceOptionCPUCacheModeDefault);
    
    for(int i = 0; i < 3; i++) {
        scene->instances[i] = device->newBuffer(sizeof(Transform) * (brickcount + ballcount + boardcount + paddlecount), MTL::ResourceOptionCPUCacheModeDefault);
    }
    
    
    scene->vertexOffsets = new std::vector<size_t> {
        0,4,8,12,16,
    };
    // we're still getting counts from here, which, maybe we shouldn't be doing
    scene->indexOffsets = new std::vector<size_t> {
//        0,6,12,18,24,
        0,0,0,0,0,
    };
    scene->indexCounts = new std::vector<size_t> {
        6,6,6,6,6,
    };
    
    // when the number of bricks changes, I'd have to modify this wouldn't I?
    scene->instanceOffsets = new std::vector<size_t> {
        0,
        ballcount,
        brickcount + ballcount,
        brickcount + ballcount + boardcount,
        brickcount + ballcount + boardcount + paddlecount,
    };
    
    // old allocations that didn't quite work...
//    size_t numVertices = scene->vertexOffsets->size();
//    size_t numIndices = scene->indexOffsets->size();
//    memcpy(scene->vertices->contents(), vertices, sizeof(MBEVertex)*scene->vertexOffsets->at(numVertices - 1));
//    memcpy(scene->indices->contents(), indices, sizeof(MBEIndex)*scene->indexOffsets->at(numIndices - 1));
    
    UpdateVertices();
    
//    memcpy(scene->vertices->contents(), vertices, sizeof(vertices));
    memcpy(scene->indices->contents(), indices, sizeof(indices));
    
    // need an array of 2D transforms, right?
    // this exists on some sort of arbitrary 2D grid
    
    // just put the screen in renderables for now
    renderables = (Renderable*)malloc(sizeof(Renderable)*renderableCount);
    Renderable blankBoard = Renderable {
        .index = 37,
        .type = BoardType,
        .show = true,
    };
    Renderable plainBrick = Renderable {
        .index = 0,
        .type = BrickType,
        .show = true,
    };
    Renderable plainBall = Renderable {
        .type = BallType,
        .show = true,
    };
    Renderable plainPaddle = Renderable {
        .type = PaddleType,
        .show = true,
    };
    
    renderables[0] = blankBoard;
    renderables[1] = plainBrick;
    renderables[2] = plainBall;
    renderables[3] = plainPaddle;
    
//    transforms = (Transform*)malloc(sizeof(Transform)*(brickcount + ballcount + boardcount + paddlecount));
    transforms = (Transform*)calloc(entitycount, sizeof(Transform));
    UpdateBaseTransforms();
//    ball = (Ball*)malloc(sizeof(Ball)*ballcount);
//    board = (Board*)malloc(sizeof(Board));
//
//    int brickHeight = 1;
//    int brickWidth = 2;
//    int brickOffset = 1;
//
//    vector_int2 ballStart = { 0, 0 };
//
//    *board = {
//        .pos = {0, 0},
//    };
    
    // some math to calculate the placement
    // init the actual game itself not the "view" of the game
//    float offset = unitSize;
//
//    // ball
//    transforms[0] = {
//        .trans =
//        {
//            0,
//            0,
//            0,
//            0,
//        }
//    };
//
//    // bricks
//    for(int i = 1; i <= brickcount; i++) {
//        float m = static_cast<float>(i);
//        transforms[i] = {
//            .trans =
//            {
////                fmod((m + 1.0f) * (offset + (static_cast<float>(brickWidth) * unitSize) / 2) + static_cast<float>(brickWidth * unitSize / 2), boardWidth - 2*unitSize),
////                unitSize * (i % 10),
//                0,
//                0,
//                0,
//                0,
//            },
//        };
//
//        transforms[i].trans.x += (halfViewWidth - halfBoardWidth); // center
//    }
//
//    // board(s)
//    for(int i = brickcount + ballcount; i < brickcount + ballcount + boardcount; i++) {
//        transforms[i] = {
//            .trans = {
//                0, 0, 0, 0,
//            }
//        };
//    }
    
    std::cout << "At 101: " << transforms[101].trans.x << "," << transforms[101].trans.y << transforms[101].trans.z << transforms[101].trans.w << std::endl;
    std::cout << "At 0: " << transforms[0].trans.x << "," << transforms[0].trans.y << transforms[0].trans.z << transforms[0].trans.w << std::endl;
//    for(int i = 0; i < ballcount; i++) {
//        ball[i] = {
//            .pos = ballStart,
//            .rot = 0.0,
//        };
//    }
    
    boardLeftX = halfViewWidth - (boardWidth / 2);
    boardRightX = halfViewWidth + (boardWidth / 2);
    boardTopY = halfViewHeight - (boardHeight / 2);
    boardBottomY = halfViewHeight + (boardHeight / 2);
    
    scene->isLoaded = true;
}

BrickBreaker::~BrickBreaker() {
    // do I free the scene?
    free(scene);
//    free(renderables);
//    free(bricks);
}

void BrickBreaker::UpdateVertices() {
    float halfViewWidth = viewWidth / 2;
    float halfViewHeight = viewHeight / 2;
    float halfBoardWidth = boardWidth / 2;
    float halfBoardHeight = boardHeight / 2;
    float externalPadding = 2*unitSize;
    float internalPadding = (fractionalBrickWidth - 1)*unitSize;
    
    
    brickWidth = (boardWidth - externalPadding - internalPadding) / fractionalBrickWidth;
    
    MBEVertex vertices[] = {
        // ball
        {.position = {0,0,0,1}, .color = {1, 1, 1, 1}},
        {.position = {0,1*unitSize,0,1}, .color = {1, 1, 1, 1}},
        {.position = {1*unitSize,1*unitSize,0,1}, .color = {1, 1, 1, 1}},
        {.position = {1*unitSize,0,0,1}, .color = {1, 1, 1, 1}},

        // brick
        {.position = {0,0,0,1},                                   .color = BRICK_COLOR},
        {.position = {0,brickHeight*unitSize,0,1},                .color = BRICK_COLOR},
        {.position = {brickWidth,brickHeight*unitSize,0,1},       .color = BRICK_COLOR},
        {.position = {brickWidth,0,0,1},                          .color = BRICK_COLOR},

        // board -> will scale correctly later
        {.position = {halfViewWidth - halfBoardWidth, halfViewHeight - halfBoardHeight, 0, 1}, .color = BOARD_COLOR},
        {.position = {halfViewWidth - halfBoardWidth, halfViewHeight + halfBoardHeight, 0, 1}, .color = BOARD_COLOR},
        {.position = {halfViewWidth + halfBoardWidth, halfViewHeight + halfBoardHeight, 0, 1}, .color = BOARD_COLOR},
        {.position = {halfViewWidth + halfBoardWidth, halfViewHeight - halfBoardHeight, 0, 1}, .color = BOARD_COLOR},

        // paddle
        {.position = {0, 0, 0, 1}, .color = {1, 1, 1, 1}},
        {.position = {0, paddleUnitHeight*unitSize, 0, 1}, .color = {1, 1, 1, 1}},
        {.position = {paddleUnitWidth*unitSize, paddleUnitHeight*unitSize, 0, 1}, .color = {1, 1, 1, 1}},
        {.position = {paddleUnitWidth*unitSize, 0, 0, 1}, .color = {1, 1, 1, 1}},
    };
    
    memcpy(scene->vertices->contents(), vertices, sizeof(vertices));
}

void BrickBreaker::UpdateBaseTransforms() {
    float offset = unitSize;
    
    int halfViewWidth = viewWidth / 2;
    int halfViewHeight = viewHeight / 2;
    int halfBoardWidth = boardWidth / 2;
    int halfBoardHeight = boardHeight / 2;
    
    float externalPadding = 2*unitSize;
    float internalPadding = (brickcount - 1) * unitSize;
    
    
    // ball
    transforms[0] = {
        .trans =
        {
            static_cast<float>(halfViewWidth - (unitSize / 2)),
            static_cast<float>(halfViewHeight - (unitSize / 2)),
            0,
            0,
        },
        .color = BRICK_COLOR
    };
//
    // bricks
    for(int i = 1; i <= brickcount; i++) {
        transforms[i] = {
            .trans =
            {
//                fmod((m + 1.0f) * (offset + (static_cast<float>(brickWidth) * unitSize) / 2) + static_cast<float>(brickWidth * unitSize / 2), boardWidth - 2*unitSize),
//                fmod(i * (brickWidth), boardWidth - externalPadding - internalPadding),
//                ((i - 1) % fractionalBrickWidth) * (unitSize + (halfViewWidth - halfBoardWidth)),
//                unitSize + (halfViewWidth - halfBoardWidth),
//                0,
                static_cast<float>(halfViewWidth - halfBoardWidth + unitSize),
                static_cast<float>(halfViewHeight - halfBoardHeight + unitSize),
                0,
                0,
            },
        };
        
        transforms[i].trans.x += (brickWidth + unitSize) * ((i - 1) % fractionalBrickWidth);
        transforms[i].trans.y += ((i - 1) / fractionalBrickWidth) * 2 * unitSize;
        transforms[i].color = BRICK_COLOR;
    }
    
    // board(s)
    for(size_t i = brickcount + ballcount; i < brickcount + ballcount + boardcount; i++) {
        transforms[i] = {
            .trans = {
                0, 0, 0, 0,
            }
        };
        transforms[i].color = BOARD_COLOR;
    }
    
    // paddle(s)
    for(size_t i = brickcount + ballcount + boardcount; i < brickcount + ballcount + boardcount + paddlecount; i++) {
        transforms[i] = {
            .trans = {
                static_cast<float>(halfViewWidth) - (paddleUnitWidth * unitSize / 2),
                static_cast<float>(halfViewHeight + (boardHeight / 4)),
                0,
                0,
            }
        };
        paddleTarget.x = transforms[i].trans.x; // fine for now, not with multiple
        transforms[i].color = BRICK_COLOR;
    }
}
    
// this would have to be after the Physics systems, etc, had their way with these...
void BrickBreaker::CopyInstanceData(int frame) {
    // for brick in bricks, etc...
    // which could change, obviously...
    // I really want a translation, rotation for every
    MTL::Buffer *destination = scene->instances[frame];
    memcpy(destination->contents(), transforms, sizeof(Transform)*entitycount);
}

void BrickBreaker::Update(double timeStep, uint16_t frameInput) {
    // there are different states
    // running
    // paused
    // loading
    // have to check for win/lose conditions, which may vary level to level
    // don't know how I'm going to deal with integer positions
//    if(frameInput == 0) {
//        return;
//    }
    
    // most likely want to lerp for smooth animation...
    float newX = static_cast<float>(timeStep * speed *
                                    (
                                     ((frameInput & LeftArrow ) >> (LeftArrow - 1)) * -1 +
                                     ((frameInput & RightArrow) >> (RightArrow - 1))
                                    )
                                   );
    newX = fmax(fmin(newX, 20.0f), -20.0f);
    
    // correct "hitch"
    // does not correct the hitch
    if((lastPaddleXUpdate < 0 && newX > 0) || (lastPaddleXUpdate > 0 && newX < 0)) {
//        paddleTarget.x = transforms[paddleIndex].trans.x;
        newX *= 2.0;
    }
    
    paddleTarget.x += newX;
    
    transforms[paddleIndex].trans = {
        lerp1D(transforms[paddleIndex].trans.x, paddleTarget.x, 0.4f),
//        newX,
        transforms[paddleIndex].trans.y,
        0,
        0,
    };
    
    lastPaddleXUpdate = newX;
    
    ballTarget.x = ballTarget.x == 0 ? boardRightX : ballTarget.x;
    ballMotion.x = ballMotion.x == 0 ? 1 : ballMotion.x;
    ballMotion.y = ballMotion.y == 0 ? -0.5 : ballMotion.y;
    
    float ballSpeed = 200;
    vector_float4 ballTransform = transforms[ballIndex].trans;
    vector_float4 paddleTransform = transforms[paddleIndex].trans;
    // update ball
    transforms[ballIndex].trans = {
//        lerp1D(transforms[ballIndex].trans.x, ballTarget.x, 0.01f),
        transforms[ballIndex].trans.x + ballMotion.x * ballSpeed * static_cast<float>(timeStep),
        transforms[ballIndex].trans.y + ballMotion.y * ballSpeed * static_cast<float>(timeStep),
        0,
        0,
    };
    
    // ball + wall collisions
    int ep = 2.0;
    if(transforms[ballIndex].trans.x >= boardRightX - ep ){
        ballTarget.x -= boardWidth;
        ballMotion.x *= -1;
    }
    else if(transforms[ballIndex].trans.x <= boardLeftX + ep){
        ballTarget.x += boardWidth;
        ballMotion.x *= -1;
    }
    
    if(transforms[ballIndex].trans.y >= boardBottomY - ep || transforms[ballIndex].trans.y <= boardTopY + ep) {
        ballMotion.y *= -1;
    }
    
    // ball + brick collisions
    ep = 2;
    
    // should I immediately display the ball?
    for(int i = 1; i <= brickcount; i++) {
        if((brickCollisions & (1 << i)) > 0) continue;
        
        if(ballTransform.y <= transforms[i].trans.y + brickHeight + ep &&
           ballTransform.y >= transforms[i].trans.y - ep               &&
           ballTransform.x >= transforms[i].trans.x - ep               &&
           ballTransform.x <= transforms[i].trans.x + brickWidth + ep) {
            // which part of the brick have we collided with
            
            // vertical collision
            if(ballTransform.x >= transforms[i].trans.x - ep && ballTransform.x <= transforms[i].trans.x + brickWidth + ep) {
                ballMotion.y *= -1;
            } else if(ballTransform.y >= transforms[i].trans.y - ep && ballTransform.y <= transforms[i].trans.y + brickHeight + ep) {
                ballMotion.x *= -1;
            }
            
            transforms[ballIndex].trans = {
                transforms[ballIndex].trans.x + ballMotion.x * ballSpeed * static_cast<float>(timeStep) * 2,
                transforms[ballIndex].trans.y + ballMotion.y * ballSpeed * static_cast<float>(timeStep) * 2,
                0,
                0,
            };
            
            brickCollisions &= (1 << i); // record collision
            
            // stop drawing the brick
            transforms[i].color = BOARD_COLOR;
        }
    }
    
    // ball + paddle collisions
    if(ballTransform.x >= paddleTransform.x - ep && ballTransform.x <= paddleTransform.x + 2 * unitSize + ep &&
       ballTransform.y >= paddleTransform.y - ep && ballTransform.y <= paddleTransform.y + unitSize + ep &&
       ballMotion.y > 0) {
        ballMotion.y *= -1;
        
        transforms[ballIndex].trans = {
            transforms[ballIndex].trans.x + ballMotion.x * ballSpeed * static_cast<float>(timeStep) * 2,
            transforms[ballIndex].trans.y + ballMotion.y * ballSpeed * static_cast<float>(timeStep) * 2,
            0,
            0,
        };
    }
}

// for updating board size etc
void BrickBreaker::UpdateSizes(float viewW, float viewH) {
    viewWidth = viewW;
    viewHeight = viewH;
    
    int halfViewWidth = viewWidth / 2;
    int halfViewHeight = viewHeight / 2;
    
    UpdateVertices();
    UpdateBaseTransforms();
    
    boardLeftX = halfViewWidth - (boardWidth / 2);
    boardRightX = halfViewWidth + (boardWidth / 2);
    boardTopY = halfViewHeight - (boardHeight / 2);
    boardBottomY = halfViewHeight + (boardHeight / 2);
}

// take a reference to the renderer
// and draw that way
// bc we have to draw the board
// we have to draw the ball(s)
// draw the bricks(s)
void BrickBreaker::Draw(Renderer *renderer) {
    // for board in boards
    // for ball in balls
    // for brick in bricks
    
    // don't really have things split into components, but this is a simple game, so I guess that wouldn't really make sense
    
    // draw board
    // get unit size based on viewport
//    vector_float2 viewport = renderer->getViewport();
    
    // I would like to be able to call
    // with the renderer handling the actual issuing of draw commands, etc.
//    renderer->drawRenderable(renderable);
    
    // need some public 2D draw for the renderer given an array of renderables
    renderer->drawRenderablesWithScene(scene, renderables, renderableCount);
}

bool BrickBreaker::IsLoaded() {
    return scene->isLoaded;
}

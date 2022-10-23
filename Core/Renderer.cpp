//
//  Renderer.cpp
//  MetalCPPSetupiOS
//
//  Created by Jordan Hall on 6/22/22.
//

#include "AAPLShaderTypes.h"


#define NS_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "CoreFoundation/CoreFoundation.h"

#include "Renderer.hpp"
#include "Scene.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // <-- should be able to use this to get image data
#include "Utils.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <simd/simd.h>
#include <math.h>


// MARK: - Destructor!!!!!

// static declarations
const char* Renderer::modelFilenames[1] {
    "spot.obj",
};

const char* Renderer::textureFilenames[1] {
    "spot.png",
};

// MARK: Todo - need a custom compare function to compare the underlying content of the char* ' s
//std::map<const char*, const char*, StrCmp> Renderer::modelFilenameToResourcePath = std::map<const char*, const char*, StrCmp> {};
std::map<std::string, std::string> Renderer::modelFilenameToResourcePath = std::map<std::string,std::string> {};
std::map<const char*, const char*, StrCmp> Renderer::textureFilenameToResourcePath = std::map<const char*, const char*, StrCmp> {};

bool Renderer::dictsLoaded = false;

class Camera
{
    
};

void LoadGeometry() {
    
};

void Input() {
    
};

const int Renderer::maxFramesInFlight = 3;


MTL::Buffer* testBuffer;



void Renderer::loadTexture(MTL::Texture *tex) {
    _pTex = tex;
}

void Renderer::setEuler(float yaw, float pitch, float roll) {
    _yaw = yaw;
    _pitch = pitch;
    _roll = roll;
}

void Renderer::setZoom(float zoom) {
    _curZoom = zoom >= _maxZoom ? _maxZoom : zoom;
}

void Renderer::togglePicture() {
    _showPicture = !_showPicture;
}

// think I may have to do something with this.
Renderer::Renderer(MTL::Device* device, NS::UInteger pixelFormatAsUInt, Scene *scene, float width, float height)
:_pDevice(device->retain()),
_frame(0)
{
    /*
     * Let's say we have a scene, which consists of a set of filepaths for now, for models
     * Later it may be files along with the descriptions of what those files are: static, character, enemy, what "functionality" the have basically
     */
    
    NS::Error* error;
    
    MTL::PixelFormat pixelFormat = (MTL::PixelFormat)pixelFormatAsUInt;
    
    std::cout << "running setup" << std::endl;
    
    _semaphore = dispatch_semaphore_create(Renderer::maxFramesInFlight);
    
    _pScene = scene;
    
    buildFrameData();
    
    // "picture texture
    // a problem here is that width * height isn't accessible in the constructor
//    _lastPicture = tex->device()->newBuffer(bytesPerImage, MTL::ResourceOptionCPUCacheModeDefault);
    
    // MARK: Todo - Rendering multiple objects
    /*
     This is call is cool(ish) for rn, however will need to change
     I'm thinking a general set of objects can be loaded, and in the draw call we draw each of the objects
     does raise the question of how we can commit to drawable once per draw call though...
     */

    // I think that the way this should work is that we should just keep appending to both, so that we do the one assignment
//    ParseOBJ(_pVertexBuffer, _pVertexIndexBuffer);
    
//    _pVertexBufferMTL = _pDevice->newBuffer(&_pVertexBuffer[0], _pVertexBuffer.size()*sizeof(MBEVertex), MTL::ResourceCPUCacheModeDefaultCache);
//    _pVertexIndexBufferMTL = _pDevice->newBuffer(&_pVertexIndexBuffer[0], _pVertexIndexBuffer.size()*sizeof(MBEIndex), MTL::ResourceCPUCacheModeDefaultCache);
//    _pVertexTextureBufferMTL = _pDevice->newBuffer(&_pVertexTextureBuffer[0], _pVertexTextureBuffer.size()*sizeof(vector_float2), MTL::ResourceCPUCacheModeDefaultCache);

    
    // want to get the device from the layer, as well as the pixel format
//    MTL::Device* pDevice = MTL::CreateSystemDefaultDevice();
    MTL::Library* pLibrary = _pDevice->newDefaultLibrary();
    MTL::Function* pVFunc = pLibrary->newFunction(NS::MakeConstantString("vertexShader"));
    MTL::Function* pFFunc = pLibrary->newFunction(NS::MakeConstantString("fragmentShader"));
    
    MTL::DepthStencilDescriptor* depthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
    depthStencilDescriptor->setDepthCompareFunction(MTL::CompareFunction::CompareFunctionLess);
    depthStencilDescriptor->setDepthWriteEnabled(true);
    
    _pDeptchStencilState = _pDevice->newDepthStencilState(depthStencilDescriptor);
    
    
    MTL::RenderPipelineDescriptor* pRenderPLDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    pRenderPLDescriptor->setLabel(NS::MakeConstantString("Triangle Render Pipeline"));
    pRenderPLDescriptor->setVertexFunction(pVFunc);
    pRenderPLDescriptor->setFragmentFunction(pFFunc);
    pRenderPLDescriptor->colorAttachments()->object(0)->setPixelFormat(pixelFormat);
    pRenderPLDescriptor->setDepthAttachmentPixelFormat( MTL::PixelFormat::PixelFormatDepth32Float );
    
    MTL::RenderPipelineState* _pPLState = _pDevice->newRenderPipelineState(pRenderPLDescriptor, &error);
    
    if ( !_pPLState )
    {
        __builtin_printf( "%s", error->localizedDescription()->utf8String() );
        assert( false );
    }

    
    MTL::CommandQueue* commandQueue = _pDevice->newCommandQueue();
    
    _pPSO = _pPLState;
    _pCommandQueue = commandQueue;
    
    // sampler
    MTL::SamplerDescriptor* samplerDes = MTL::SamplerDescriptor::alloc()->init();
    samplerDes->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDes->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDes->setMinFilter(MTL::SamplerMinMagFilterNearest);
    samplerDes->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDes->setMipFilter(MTL::SamplerMipFilterLinear);
    _pSamplerState = _pDevice->newSamplerState(samplerDes);
    
//    testBuffer = _pDevice->newBuffer(pictureVertices, 4*sizeof(MBEVertex), MTL::ResourceCPUCacheModeDefaultCache);
    
    
//    free(data);
    pVFunc->release();
    pFFunc->release();
    pRenderPLDescriptor->release();
    pLibrary->release();
    
    std::cout << "done with setup" << std::endl;
};

void Renderer::delegateTestRun(){
    ((RenderDelegate *)delegateTest)->run();
}

void Renderer::addModel(std::string modelPath){
    _modelPaths.push_back(modelPath);
};

void Renderer::buildFrameData() {
    for(int i = 0; i < Renderer::maxFramesInFlight; i++) {
        _pFrameData[i] = _pDevice->newBuffer(sizeof(FrameData), MTL::ResourceCPUCacheModeDefaultCache);
    }
};

uint32_t Renderer::sizeOfPixelFormat(NS::UInteger format) {
    return ((format) == MTL::PixelFormatBGRA8Unorm ? 4 :
            (format) == MTL::PixelFormatR32Uint    ? 4 : 0);
}

void Renderer::takePicture() {
    _takePicture = !_takePicture;
}

void Renderer::copyToPictureBuffer(MTL::Texture *tex, MTL::CommandBuffer *cmdBuffer) {
    if(!_takePicture) return;
    
    NS::UInteger format = sizeOfPixelFormat(tex->pixelFormat());
    NS::UInteger bytesPerRow = tex->width() * format;
    NS::UInteger bytesPerImage = tex->height() * bytesPerRow;
    
    // a problem here is that width * height isn't accessible in the constructor
//    _lastPicture = tex->device()->newBuffer(bytesPerImage, MTL::ResourceOptionCPUCacheModeDefault);
    
    
    
    // MARK: Todo - copy texture to buffer
    
    MTL::BlitCommandEncoder *blitCommandEncoder = cmdBuffer->blitCommandEncoder();
    
    // origin
    MTL::Origin origin = MTL::Origin(0, 0, 0); // is this correct?
    
    // actual nightmare function
    MTL::Size size = MTL::Size(tex->width(), tex->height(), tex->depth());
    
//    blitCommandEncoder->copyFromTexture(tex, 0, 0, origin, size, _lastPicture, 0, bytesPerRow, bytesPerImage);
    
    blitCommandEncoder->copyFromTexture(tex, _lastPicture);
    
    blitCommandEncoder->endEncoding();
    
    _takePicture = false;
}

void Renderer::displayPictureBuffer() {
    
}

void Renderer::setFrameData(matrix_float4x4 modelMatrix, vector_float4 color)
{
    // what we're going to do is...
    // is set up the modelviewproj matrix, and then, use it...
    // just going to translate and project for now...
    
    vector_float3 cameraTranslate = {0, 0, -5};
        
    matrix_float4x4 viewMatrix = _lookat;
//    matrix_float4x4 viewMatrix = matrix_float4x4_translation(cameraTranslate);
    matrix_float4x4 scaleMatrix = matrix_float4x4_uniform_scale(1.0);
    float aspect = (float)_viewportSize.x/(float)_viewportSize.y;
    float fov = 2 * M_PI / 5;
    float near = 1;
    float far = 100;
    
//    vector_float3 verticalShift = {0,(float)((-_roll / (M_PI))) * 10.0f, 0};
//    vector_float3 horizontalShift = {(float)((-_pitch / (M_PI))) * 20.0f, 0, 0};
//    viewMatrix = matrix_multiply(viewMatrix, matrix_float4x4_translation(verticalShift));
//    viewMatrix = matrix_multiply(viewMatrix, matrix_float4x4_translation(horizontalShift));
//
    
    matrix_float4x4 projMatrix = matrix_float4x4_perspective(aspect, fov, near, far);
//    matrix_float4x4 projMatrix = matrix_identity_float4x4;
    matrix_float4x4 mv = matrix_multiply(viewMatrix, matrix_multiply(scaleMatrix, modelMatrix));
    matrix_float3x3 normalMatrix = matrix_float4x4_extract_linear(mv);
    matrix_float4x4 mvp = matrix_multiply(projMatrix, matrix_multiply(viewMatrix, matrix_multiply(scaleMatrix, modelMatrix)));
    
    FrameData* data = new FrameData();
//    data->modelViewProjMatrix = matrix_identity_float4x4;
//    data->modelViewMatrix = matrix_identity_float4x4;
    data->modelViewProjMatrix = mvp;
    data->modelViewMatrix = mv;
//    data->modelViewProjMatrix = matrix_identity_float4x4;
//    data->modelViewMatrix = matrix_identity_float4x4;
    data->normalMatrix = normalMatrix;
    
    data->color = color;
    
    MTL::Buffer* frameDataBuffer = _pDevice->newBuffer(data, sizeof(FrameData), MTL::ResourceCPUCacheModeDefaultCache);
    
    _pFrameData[_frame] = frameDataBuffer;
};

Renderer::~Renderer()
{
    _pVertices->release();
    _pColors->release();
    _pVertexBufferMTL->release();
    _pVertexIndexBufferMTL->release();
    _pPSO->release();
    _pCommandQueue->release();
    _pDevice->release();
}

bool Renderer::drawCurrentScene(MTL::RenderCommandEncoder *rce) {
    // for each model in the scene
    // set the vertex buffer
    // MARK: Todo - Can I just pass the entire vertex buffer
    
    if(_pScene == nullptr) return false;
    
    for(int i = 0; i < _pScene->modelIndices.size(); i++) {
        rce->setVertexBuffer(_pScene->vertices, _pScene->vertexOffsets[i], 0); // this should be changed to actuall y reflect how the shader works
// set buffer for the transformations
        
        // how do we want to handle transformations here
        // I suppose that that's entirely external to the task of "rendering"
//        rce->setVertexBuffer(_pScene->transforms, i*sizeof(matrix_float4x4), 1); // transforms will all be the same size
    }
    
    return true;
}

// set up current command buffer
void Renderer::startDraw(MTL::RenderPassDescriptor* renderPassDesc, CA::MetalDrawable* drawable) {
    _pool = NS::AutoreleasePool::alloc()->init();
    
    // let's also do this at a scale of 250
    // update uniforms as need be
    _frame = (_frame + 1) % Renderer::maxFramesInFlight;
    
    // ======================= SETUP ============================
    _drawable = drawable;
    _commandBuffer = _pCommandQueue->commandBuffer();
    dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
    
    Renderer* ptrRenderer = this;
    _commandBuffer->addCompletedHandler(^void(MTL::CommandBuffer *buffer) {
        dispatch_semaphore_signal(ptrRenderer->_semaphore);
    });
    
    _commandBuffer->setLabel(NS::MakeConstantString("commandBuffer"));
    renderPassDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0.0, 0.5, 0.7, 1.0));

    _renderCommandEncoder = _commandBuffer->renderCommandEncoder(renderPassDesc);
    
    _renderCommandEncoder->setLabel(NS::MakeConstantString(("mycommandencoder")));
    _renderCommandEncoder->setCullMode(MTL::CullMode::CullModeBack);

    //    renderCommandEncoder->setTriangleFillMode(MTL::TriangleFillMode::TriangleFillModeLines);
    _renderCommandEncoder->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);
    _renderCommandEncoder->setDepthStencilState(_pDeptchStencilState);

    _renderCommandEncoder->setRenderPipelineState(_pPSO);
    // ==========================================================
}

// commit the command buffer
void Renderer::endDraw() {
    _renderCommandEncoder->endEncoding();
    _commandBuffer->presentDrawable(_drawable);
    _commandBuffer->commit();
    
    _pool->release();
}

void Renderer::addDrawCommands(size_t vertexOffset, size_t indexOffset, size_t indexCount, MTL::Texture* texture, vector_float4 color, matrix_float4x4 transform){
    // frame data
    setFrameData(transform, color);
    
    if(!_pScene->isLoaded) {
        // present some kind of basic view
        
//        _renderCommandEncoder->setVertexBuffer(_pScene->vertices, vertexOffset, 0);
//        
//        _renderCommandEncoder->setVertexBuffer(_pFrameData[_frame], 0, 1);
//        // encode fragment data
//        _renderCommandEncoder->setFragmentTexture(texture, 0);
//        _renderCommandEncoder->setFragmentSamplerState(_pSamplerState, 0);
//
//        _renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
//                                                    indexCount, // <- idk know about this
//                                                    MBEIndexType,
//                                                    _pScene->indices,
//                                                    indexOffset);
        
        return;
    }
    
    // encode vertex data
    _renderCommandEncoder->setVertexBuffer(_pScene->vertices, vertexOffset, 0);
    
    // the renderer should actually deal with mvp and mv matrices
//    _renderCommandEncoder->setVertexBytes(_pFrameData[_frame], sizeof(FrameData), 1);
    _renderCommandEncoder->setVertexBuffer(_pFrameData[_frame], 0, 1);
    // encode fragment data
    _renderCommandEncoder->setFragmentTexture(texture, 0);
    _renderCommandEncoder->setFragmentSamplerState(_pSamplerState, 0);

    _renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                                indexCount, // <- idk know about this
                                                MBEIndexType,
                                                _pScene->indices,
                                                indexOffset);
}

// can this just not be done on the gpu?
void Renderer::updateLookat(float deltaX, float deltaY) {
    // implement lookat matrix
    // we're gonna stick to the origin for now...
    // muy muy jittery
    
    float dy = 0.01;
    
    vector_float3 target = {0, 0, 0};
    vector_float3 up = {0, 1, 0};
    float radius = 2.0f;
    
    // derive right and viewing direction from up
    vector_float3 right;
    vector_float3 viewingDir;
    
    // how to keep this angle from getting too big?
//    double yaw = (_prevYaw + M_PI * (deltaX / _viewportSize.x));
//    double pitch = (_prevPitch + (M_PI / 2) * (deltaY /_viewportSize.y));
    
    double yaw = _prevYaw + 0.01;
    
    //    double pitch = _prevPitch + dy;
    //    double angle = _prevAngle + deltaX;
//    if(pitch <= -(M_PI / 2)) {
//        pitch = (M_PI / 180.0) * -89.0;
//    }
//    if(pitch >= (M_PI / 2)) {
//        pitch = (M_PI / 180.0) * 89.0;
//    }
    
//    std::cout << "yaw: " << yaw << std::endl;
//    std::cout << "pitch: " << pitch << std::endl;
    
//    _prevYaw = fmod(yaw, 2 * M_PI);
//    _prevPitch = pitch; // might mod by pi
    _prevYaw = yaw;
    
    vector_float3 pos;
    
    pos = {(float)sin(yaw) * radius, 0, (float)cos(yaw) * radius};
    
//    pos = {(float)cos(yaw)*(float)cos(pitch)*radius, (float)sin(pitch)*radius, (float)sin(yaw)*(float)cos(pitch)*radius};
    
//    if(abs(deltaX) > abs(deltaY)) {
//        pos = {(float)cos(yaw) * radius, 0, (float)sin(yaw) * radius};
//    } else {
//        
//    }
    
    viewingDir = pos - target;
    right = simd_cross(viewingDir, up); // is this right or left?
    
    vector_float4 X = {right.x, up.x, viewingDir.x, 0};
    vector_float4 Y = {right.y, up.y, viewingDir.y, 0};
    vector_float4 Z = {right.z, up.z, viewingDir.z, 0};
    vector_float4 W = {0, 0, 0, 1};
    
    matrix_float4x4 lhs = {X, Y, Z, W};
    
    
    _lookat = matrix_multiply(lhs, matrix_float4x4_translation(-pos));
}

void Renderer::draw(MTL::RenderPassDescriptor* renderPassDesc, CA::MetalDrawable* drawable, NS::Integer preferredFramesPerSecond)
{
//    std::cout << "drawing in renderer" << std::endl;
    
    NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
    
    // let's also do this at a scale of 250
    // update uniforms as need be
    _frame = (_frame + 1) % Renderer::maxFramesInFlight;
//    setupUniforms(preferredFramesPerSecond);
    
    // ======================= SETUP ============================
    MTL::CommandBuffer* commandBuffer = _pCommandQueue->commandBuffer();
    dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);
    
    Renderer* ptrRenderer = this;
    commandBuffer->addCompletedHandler(^void(MTL::CommandBuffer *buffer) {
        dispatch_semaphore_signal(ptrRenderer->_semaphore);
    });
    
    commandBuffer->setLabel(NS::MakeConstantString("commandBuffer"));
    renderPassDesc->colorAttachments()->object(0)->setClearColor(MTL::ClearColor::Make(0.0, 0.5, 0.7, 1.0));

    MTL::RenderCommandEncoder* renderCommandEncoder = commandBuffer->renderCommandEncoder(renderPassDesc);
    
    renderCommandEncoder->setLabel(NS::MakeConstantString(("mycommandencoder")));
    renderCommandEncoder->setCullMode(MTL::CullMode::CullModeBack);
//    renderCommandEncoder->setTriangleFillMode(MTL::TriangleFillMode::TriangleFillModeLines);
    renderCommandEncoder->setFrontFacingWinding(MTL::Winding::WindingCounterClockwise);
    renderCommandEncoder->setDepthStencilState(_pDeptchStencilState);

    renderCommandEncoder->setRenderPipelineState(_pPSO);
    // ==========================================================
    
    
    // MARK: Loop through objects in scene
    
    // encode vertex data
    renderCommandEncoder->setVertexBuffer(_pVertexBufferMTL, 0, 0);
    renderCommandEncoder->setVertexBuffer(_pFrameData[_frame], 0, 1);

    // encode fragment data
    renderCommandEncoder->setFragmentTexture(_pTex, 0);
    renderCommandEncoder->setFragmentSamplerState(_pSamplerState, 0);

    renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle,
                                                _pVertexIndexBufferMTL->length() / sizeof(MBEIndex), // <- idk know about this
                                                MBEIndexType,
                                                _pVertexIndexBufferMTL,
                                                0);

    // should I provide an empty texture?
    // the frame data should be set up with identity matrices, right now, we see nothing
    
    // frame data for the "picture"
    FrameData* data = new FrameData();
    data->modelViewProjMatrix = matrix_identity_float4x4;
    data->modelViewMatrix = matrix_identity_float4x4;
    data->normalMatrix = matrix_identity_float3x3;

    MBEVertex pictureVertices[] = {
        {
            .position = {0.8, 0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, 1.},
            .uv = {0, 0}
        },
        {
            .position = {-0.8, 0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, .1},
            .uv = {0, 0}
        },
        {
            .position = {0.8, -0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, 1.},
            .uv = {0, 0}
        },
        {
            .position = {-0.8, 0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, 1.},
            .uv = {0, 0}
        },
        {
            .position = {-0.8, -0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, 1.},
            .uv = {0, 0}
        },
        {
            .position = {0.8, -0.8, 0, 1},
            .normal = {0, 0, -1, 0},
            .color = {1., 0, 0, 1.},
            .uv = {0, 0}
        },
    };
    
//    MBEIndex indexBuffer[] = {
//        0,1,2
//    };
    
//    MTL::Buffer *picIndexBuffer = _pDevice->newBuffer(indexBuffer, NS::UInteger(6) * sizeof(MBEIndex), MTL::ResourceOptionCPUCacheModeDefault);
    MTL::Buffer *posBuffer = _pDevice->newBuffer(pictureVertices, sizeof(pictureVertices), MTL::ResourceOptionCPUCacheModeDefault);
   
    // is you just use draw primitives, the order of the points matters. It's just going to assume
    // you've given it the points in the order of counterclockwise triangles
    if(_showPicture){
        // setup picture texture
        renderCommandEncoder->setFragmentTexture(_lastPicture, 0);
        renderCommandEncoder->setFragmentSamplerState(_pSamplerState, 0);

        MTL::Buffer *identityFrameBuffer = _pDevice->newBuffer(data, sizeof(FrameData), MTL::ResourceOptionCPUCacheModeDefault);
        
        renderCommandEncoder->setVertexBuffer(posBuffer, 0, 0);
        renderCommandEncoder->setVertexBuffer(identityFrameBuffer, 0, 1);
        
        renderCommandEncoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, 0, 6, 1);
    }
    
//    renderCommandEncoder->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, picIndexBuffer->length() / sizeof(MBEIndex), MBEIndexType, picIndexBuffer, NS::UInteger(0));
    
    renderCommandEncoder->endEncoding();
    
    copyToPictureBuffer(drawable->texture(), commandBuffer); // conditional
    
    commandBuffer->presentDrawable(drawable); // not const
    commandBuffer->commit();
    
    // capture frame?
    // but then, when would I display the texture?
    // but the texture on a plane and just go...?
    
    pPool->release();
};

void Renderer::setViewportSize(uint x, uint y)
{
    _viewportSize.x = x;
    _viewportSize.y = y;
}



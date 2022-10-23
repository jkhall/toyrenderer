//
//  Renderer.hpp
//  MetalCPPSetupiOS
//
//  Created by Jordan Hall on 6/22/22.
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "AAPLShaderTypes.h"
#include <stdio.h>
#include <simd/simd.h>
#include <vector>
#include <objc/runtime.h>
#include "Scene.hpp"
#include "Utils.h"
#include <map>

const MTL::IndexType MBEIndexType = MTL::IndexTypeUInt16;

class Renderer
{
    public:
        Renderer(MTL::Device*, NS::UInteger, Scene *scene, float width, float height);
        ~Renderer();
        void draw(MTL::RenderPassDescriptor* renderPassDesc, CA::MetalDrawable* drawable, NS::Integer preferredFramesPerSecond);
        bool drawCurrentScene(MTL::RenderCommandEncoder*);
        void startDraw(MTL::RenderPassDescriptor*, CA::MetalDrawable*);
        void endDraw();
        void addDrawCommands(size_t vertexOffset, size_t indexOffset, size_t indexCount, MTL::Texture* texture, vector_float4 color, matrix_float4x4 transform);
        void setFrameData(matrix_float4x4, vector_float4);
        void setViewportSize(uint x, uint y);
        void addModel(std::string modelPath);
        void buildFrameData();
        void* delegateTest;
        void delegateTestRun();
        static const int maxFramesInFlight;
        void loadTexture(MTL::Texture* tex);
        void setEuler(float yaw, float pitch, float roll);
        void setZoom(float zoom); // can i convert cgfloat to float?
        void updateLookat(float deltaX, float deltaY);
        void copyToPictureBuffer(MTL::Texture* tex, MTL::CommandBuffer *commandBuffer); // get's texture ready for display
        void displayPictureBuffer();
        
        static bool dictsLoaded;
    
        void togglePicture();
        void takePicture();
        static uint32_t sizeOfPixelFormat(NS::UInteger format);
        
        static std::map<std::string, std::string> modelFilenameToResourcePath;
        static std::map<const char*, const char*, StrCmp> textureFilenameToResourcePath;
        static const char* modelFilenames[1];
        static const char* textureFilenames[1];

    private:
        MTL::Device* _pDevice;
        MTL::CommandQueue *_pCommandQueue;
        MTL::RenderPipelineState* _pPSO; // pipeline state object
        MTL::Buffer* _pVertices;
        MTL::Buffer* _pColors;
        MTL::Buffer* _pCubeVertices;
        MTL::Buffer* _pCubeIndices;
        MTL::DepthStencilState* _pDeptchStencilState;
        //MTL::Buffer* _pModelViewProjMat[3];
        MTL::Buffer* _pFrameData[3];
        MTL::Buffer* _pVertexIndexBufferMTL;
        MTL::Buffer* _pVertexBufferMTL;
        MTL::Buffer* _pVertexTextureBufferMTL;
        MTL::Texture* _lastPicture;
    
        MTL::Texture* _pTex;
        MTL::SamplerState* _pSamplerState;
        
        NS::AutoreleasePool* _pool;
        MTL::CommandBuffer* _commandBuffer;
        MTL::RenderCommandEncoder* _renderCommandEncoder;
        CA::MetalDrawable* _drawable;
        // Scene
        Scene *_pScene;
    
        // have to have a think about how to represent this
        std::vector<MBEVertex> _pVertexBuffer;
        std::vector<vector_float2> _pVertexTextureBuffer;
        std::vector<MBEIndex> _pVertexIndexBuffer;
        std::vector<MBEIndex> _pVertexTextureIndexBuffer;
        std::vector<MBEIndex> _pVertexNormalIndexBuffer;
        std::vector<std::string> _modelPaths;
        std::vector<MTL::Buffer*> _modelVertices;
        std::vector<MTL::Buffer*> _modelIndices;
        
        // going to want to flesh out how the "scene" will be structured as well
        // could just be json
        // that seems, reasonable
        
        std::vector<size_t> _objects; // the number in here can correspond to the model
        std::vector<vector_float3> _positions;
        std::vector<float> _scales;
    
        MTL::Buffer* _pictureBuffer;
    
        char* meshNames[5];
        
        float _yaw;
        float _pitch;
        float _roll;
        
        float _rotX;
        float _rotY;
        
        // camera
        matrix_float4x4 _lookat;
        double _prevYaw = 0.0;
        double _prevPitch = 0.0;
    
        vector_uint2 _viewportSize;
        dispatch_semaphore_t _semaphore;
        int _frame;
        float _curZoom = 1.0f;
    
        bool _showPicture = false;
        bool _takePicture = false; // probably should make sure that only one thread can set this
    
    
        // constants
        const float _maxZoom = 10.0f;
        
};

class RenderDelegate {
public:
    RenderDelegate(){};
    ~RenderDelegate(){};
    virtual void run();
};

#endif /* Renderer_hpp */

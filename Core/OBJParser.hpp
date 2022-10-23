//
//  OBJParser.hpp
//  MetalCPPiOSSetup2
//
//  Created by Jordan Hall on 7/17/22.
//
#ifndef OBJParser_hpp
#define OBJParser_hpp

#include "AAPLShaderTypes.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <Metal/Metal.hpp>
#include <vector>
#include <map>
#include <simd/simd.h>
#include "Renderer.hpp"

/*
 I actually think that this is better off turning into some sort of resource management class
 */

static char* getChunk(char** src, char* delim) {
    return strsep(src, delim);
}

typedef struct {
    float x;
    float y;
    float z;
} Normal;

typedef struct {
    int vi;
    int ti;
    int ni;
} FaceVertex;

typedef struct {
    std::vector<MBEVertex> vertexData;
    std::vector<MBEIndex> indexData;
} FileModelData;


std::vector<vector_float4> _vertices;
std::vector<vector_float4> _normals;
std::vector<vector_float2> _texCoords;

static bool operator<(const FaceVertex& v0, const FaceVertex& v1) {
    if (v0.vi < v1.vi)
        return true;
    if (v0.vi > v1.vi)
        return false;
    if (v0.ti < v1.ti)
        return true;
    if (v0.ti > v1.ti)
        return false;
    if(v0.ni < v1.ni)
        return true;
    if(v0.ni > v1.ni)
        return false;
    return false;
}

void addFaceVertex(FaceVertex fv, std::vector<MBEVertex>& vertices, std::vector<MBEIndex>& indices, std::map<FaceVertex,int>& indexMap) {
    auto it = indexMap.find(fv);
    
    if(it != indexMap.end()) {
        indices.push_back((*it).second);
    } else {
        // new vertex and index
        MBEVertex vertex;
        
        vertex.position = _vertices[fv.vi];
        vertex.uv = _texCoords[fv.ti];
        vertex.normal = _normals[fv.ni];
        
        vertices.push_back(vertex);
        int idx = (int)(vertices.size() - 1);
        indices.push_back(idx);
        indexMap[fv] = idx;
    }
}

// this should take an offset, right?
// why just manipulate vectors
// and copy data to metal buffers elsewhere
FileModelData* AddModelDataFromFile(std::string fileName)
{
    /*
     This should parse directly into metal buffers, avoiding copying from vectors to metal buffers
     */
    std::vector<MBEVertex> vertexBuffer = std::vector<MBEVertex> {};
    std::vector<MBEIndex> vertexIndexBuffer = std::vector<MBEIndex> {};
    std::fstream objFile;
    objFile.open(fileName, std::ios::in);
    std::string line;
    
    std::cout << "Parsing" << std::endl;
    std::cout << "vertex buffer start size: " << vertexBuffer.size() << std::endl;
    
    if(objFile.is_open()) {
        std::cout << "file opened successfully" << std::endl;
    } else {
        std::cout << "file not opened successfully" << std::endl;
        return nullptr;
    }
    
    std::string ident;
    int vertexNormalIndex = 0;

    int vcount = 0, vncount = 0;
    float x, y, z;
    float nx, ny, nz;
    int vi = 0, vti = 0, vni = 0;
    char* f1 = new char[100];
    char* f2 = new char[100];
    char* f3 = new char[100];
    
    std::map<FaceVertex, int> indexMap = {};
    
    while(objFile >> ident) {
        switch(ident.size()) {
            case 1:
                if(ident == "v") {
                    vcount++;
                    objFile >> x;
                    objFile >> y;
                    objFile >> z;
                    // will this automatically cast?
                    // MBEVertex vertex = {.position={x, y, z, 1.0}, .color={1.0, 1.0, 1.0, 1.0}, .normal={0.0, 0.0, 0.0, 0.0}};
                    _vertices.push_back((vector_float4){x,y,z,1.0});
                } else if(ident == "f") {
                    objFile >> f1;
                    objFile >> f2;
                    objFile >> f3;
                    
                    // don't know if this will work at all... it sorta did but vertices on faces don't necessarily refer to same index in the arrays
                    // also, in the teapot's case, there are more normals than vertices
                    char* pf1 = f1;
                    char* pf2 = f2;
                    char* pf3 = f3;
                    
//                    strcpy(pf1, f1.c_str());
//                    strcpy(pf2, f2.c_str());
//                    strcpy(pf3, f3.c_str());
                    
//                    pf1 = &*f1.begin();
//                    pf2 = &*f2.begin();
//                    pf3 = &*f3.begin();
                    
//                    std::cout << "vt size: " << vertexTextureBuffer.size() << std::endl;
//                    std::cout << "v size: " << vertexBuffer.size() << std::endl;
                    
                    vi = atoi(getChunk(&pf1, (char*)"/"));
                    vi = (vi < 0) ? (int)_vertices.size() + vi - 1 : vi - 1;
                    vti = atoi(getChunk(&pf1, (char*)"/"));
                    vti = (vti < 0) ? (int)_texCoords.size() + vti - 1 : vti - 1;
                    vni = atoi(getChunk(&pf1, (char*)"/"));
                    vni = (vni < 0) ? (int)_normals.size() + vni - 1: vni - 1;
                    
//                    add this face vertex
                    addFaceVertex({.vi=vi,.ti=vti,.ni=vni}, vertexBuffer, vertexIndexBuffer, indexMap);
//                    std::cout << "vi: " << vi << ", vti: " << vti << ", vni: " << vni << std::endl;
                    
//                    vertexIndexBuffer.push_back((MBEIndex)vi);
//                    vertexTextureIndexBuffer.push_back((MBEIndex)vti);
//                    vertexNormalIndexBuffer.push_back((MBEIndex)vni);
//
//                    vertexBuffer[vi].normal = vni != INVALID_INDEX ? normals[vni] : UP;
//                    vertexBuffer[vi].uv = vertexTextureBuffer[vti];
                    
                    vi = atoi(getChunk(&pf2, (char*)"/"));
                    vi = (vi < 0) ? (int)_vertices.size() + vi - 1 : vi - 1;
                    vti = atoi(getChunk(&pf2, (char*)"/"));
                    vti = (vti < 0) ? (int)_texCoords.size() + vti - 1 : vti - 1;
                    vni = atoi(getChunk(&pf2, (char*)"/"));
                    vni = (vni < 0) ? (int)_normals.size() + vni - 1: vni - 1;
                    
                    addFaceVertex({.vi=vi,.ti=vti,.ni=vni}, vertexBuffer, vertexIndexBuffer, indexMap);
//                    vertexIndexBuffer.push_back((MBEIndex)vi);
//                    vertexTextureIndexBuffer.push_back((MBEIndex)vti);
//                    vertexNormalIndexBuffer.push_back((MBEIndex)vni);
//
//                    vertexBuffer[vi].normal = vni != INVALID_INDEX ? normals[vni] : UP;
//                    vertexBuffer[vi].uv = vertexTextureBuffer[vti];
//
                    vi = atoi(getChunk(&pf3, (char*)"/"));
                    vi = (vi < 0) ? (int)_vertices.size() + vi - 1 : vi - 1;
                    vti = atoi(getChunk(&pf3, (char*)"/"));
                    vti = (vti < 0) ? (int)_texCoords.size() + vti - 1 : vti - 1;
                    vni = atoi(getChunk(&pf3, (char*)"/"));
                    vni = (vni < 0) ? (int)_normals.size() + vni - 1 : vni - 1;
                    
                    addFaceVertex({.vi=vi,.ti=vti,.ni=vni}, vertexBuffer, vertexIndexBuffer, indexMap);
                    
//                    vertexIndexBuffer.push_back((MBEIndex)vi);
//                    vertexTextureIndexBuffer.push_back((MBEIndex)vti);
//                    vertexNormalIndexBuffer.push_back((MBEIndex)vni);
//
//                    vertexBuffer[vi].normal = vni != INVALID_INDEX ? normals[vni] : UP;
//                    vertexBuffer[vi].uv = vertexTextureBuffer[vti];
//                    vertexIndexBuffer[vertexIndexBuffer.size() - 1] = vertexIndexBuffer[vertexIndexBuffer.size() - 1] - 1;
//                    vertexTextureIndexBuffer[vertexTextureIndexBuffer.size() - 1] = vertexTextureIndexBuffer[vertexTextureIndexBuffer.size() - 1] - 1;
//                    vertexNormalIndexBuffer[vertexNormalIndexBuffer.size() - 1] = vertexNormalIndexBuffer[vertexNormalIndexBuffer.size() - 1] - 1;
                    
//                    int i = vertexIndexBuffer.back();
//                    int j = vertexNormalIndexBuffer.back();
//
//                    vertexBuffer[vertexIndexBuffer[i]].normal = vector4(a.x, a.y, a.z, 1.0f);
//                    vertexBuffer[vertexIndexBuffer[i - 1]].normal = vector4(b.x, b.y, b.z, 1.0f);
//                    vertexBuffer[vertexIndexBuffer[i - 2]].normal = vector4(c.x, c.y, c.z, 1.0f);
                }
                else if(ident == "g") {
                    std::cout << "got group" << std::endl;
                }
                else if(atoi(&ident[0]) == EOF){
                    std::cout << "end of file" << std::endl;
                };
                break;
            case 2:
                if(ident == "vn") {
                    vncount++;
                    objFile >> nx;
                    objFile >> ny;
                    objFile >> nz;
                    
                    vector_float4 nv = {nx, ny, nz, 0.0f};
                    _normals.push_back(nv);
                        
//                    vertexBuffer[vertexNormalIndex].normal.x = nx;
//                    vertexBuffer[vertexNormalIndex].normal.y = ny;
//                    vertexBuffer[vertexNormalIndex].normal.z = nz;
////                    vertexBuffer[vertexNormalIndex].normal.w = 1.0;
                    
                    vertexNormalIndex++;
                    
                } else if(ident == "vt") {
                    // have to grab texture coordinates for spot...ugh
                    objFile >> nx;
                    objFile >> ny;
                    
                    if(nx < 0) std::cout << "negative vt x: " << nx << std::endl;
                    if(ny < 0) std::cout << "negative vt y: " << ny << std::endl;
                    
                    vector_float2 v {nx, ny};
//                    std::cout << "vt.x: " << v.x << ", vt.y: " << v.y << std::endl;
//                    vertexTextureBuffer.push_back(v);
                    _texCoords.push_back(v);
                } else if(ident == "vp") {
                    
                }
                break;
            default:
                break;
        }
    }
    
    std::cout << "raw vcount: " << _vertices.size() << std::endl;
    std::cout << "vertex count: " << vertexBuffer.size() << std::endl;
//    std::cout << "vertex count: " << vcount << std::endl;
//    std::cout << "vertex normal count: " << vncount << std::endl;
    std::cout << "vertex index count: " << vertexIndexBuffer.size() << std::endl;
    
    // copy over to metal buffers
    FileModelData *data = (FileModelData*)malloc(sizeof(FileModelData));
    data->indexData = vertexIndexBuffer;
    data->vertexData = vertexBuffer;
    
    return data;
}

static void LoadModelDataFromScene(Scene *scene, MTL::Device *device) {
    // how will we manage the offsets into the metal buffers
    // can you check that a pointer is unitialized?
    // or will we just say, okay we're gonna set things into this bad boy
    
    // when is the map from the renderer loaded? That'd have to be in a particular order, right?
    // it would make more sense if we knew that the map was going
    std::vector<FileModelData*> allData = std::vector<FileModelData*>();
    size_t vertexByteCount = 0;
    size_t indexByteCount = 0;
    
    bool areDictsLoaded = Renderer::dictsLoaded;
    
    for(auto filename : scene->rawModelNames) {
        if(filename == NULL) break;
        
        // do you only need this crazy stuff when working on iOS?
        auto search = Renderer::modelFilenameToResourcePath.find(filename);
        if(search == Renderer::modelFilenameToResourcePath.end()){
            return;
        }
        
        std::string str = Renderer::modelFilenameToResourcePath[filename];
        FileModelData *data = AddModelDataFromFile(str);
        allData.push_back(data);
        vertexByteCount += data->vertexData.size();
        indexByteCount += data->indexData.size();
    }
    
    
    scene->vertices = device->newBuffer(vertexByteCount*sizeof(MBEVertex), MTL::ResourceOptionCPUCacheModeDefault);
    scene->indices = device->newBuffer(indexByteCount*sizeof(MBEIndex), MTL::ResourceOptionCPUCacheModeDefault);
    
    MTL::Buffer *vertexStart = scene->vertices;
    MTL::Buffer *indexStart = scene->indices;
    
    size_t vertexOffset = 0;
    size_t indexOffset = 0;
    
    scene->vertexOffsets = std::vector<size_t> {};
    scene->indexOffsets = std::vector<size_t> {};
    
    for(auto &modelData : allData){
        // record offsets
        scene->vertexOffsets.push_back(vertexOffset);
        scene->indexOffsets.push_back(indexOffset);
        
        // copy data from vectors to mtl buffers
        memcpy(scene->vertices->contents(), &modelData->vertexData[0], modelData->vertexData.size()*sizeof(MBEVertex));
        memcpy(scene->indices->contents(), &modelData->indexData[0], modelData->indexData.size()*sizeof(MBEIndex));
        
        // advance pointers
        scene->vertices += modelData->vertexData.size()*sizeof(MBEVertex);
        scene->indices += modelData->indexData.size()*sizeof(MBEIndex);
        
        // advance offsets
        vertexOffset += modelData->vertexData.size()*sizeof(MBEVertex);
        indexOffset += modelData->indexData.size()*sizeof(MBEIndex);
    }
    
    // copy over remaining offsets;
    scene->vertexOffsets.push_back(vertexOffset);
    scene->indexOffsets.push_back(indexOffset);
    
    // reset pointers
    scene->vertices = vertexStart;
    scene->indices = indexStart;
}

#endif

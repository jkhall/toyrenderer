/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Header containing types and enum constants shared between Metal shaders and C/ObjC source
*/

#ifndef AAPLShaderTypes_h
#define AAPLShaderTypes_h

#include <simd/simd.h>

// Buffer index values shared between shader and C code to ensure Metal shader buffer inputs
// match Metal API buffer set calls.
typedef enum AAPLVertexInputIndex
{
    AAPLVertexInputIndexVertices     = 0,
    AAPLVertexInputIndexViewportSize = 1,
} AAPLVertexInputIndex;

//  This structure defines the layout of vertices sent to the vertex
//  shader. This header is shared between the .metal shader and C code, to guarantee that
//  the layout of the vertex array in the C code matches the layout that the .metal
//  vertex shader expects.
typedef struct
{
    vector_float2 position;
    vector_float4 color;
} AAPLVertex;

typedef struct
{
    vector_float4 position;
    vector_float4 normal;
    vector_float4 color;
    vector_float2 uv;
} MBEVertex;

typedef uint16_t MBEIndex;

typedef struct {
    matrix_float4x4 modelViewProjMat;
} MBEUniforms;

static matrix_float4x4 matrix_float4x4_translation(vector_float3 t) {
    vector_float4 X = {1, 0, 0, 0};
    vector_float4 Y = {0, 1, 0, 0};
    vector_float4 Z = {0, 0, 1, 0};
    vector_float4 W = {t.x,t.y,t.z,1};
    
    matrix_float4x4 mat = {X, Y, Z, W};
    return mat;
};

static matrix_float4x4 matrix_float4x4_uniform_scale(float scale) {
    vector_float4 X = {scale, 0, 0, 0};
    vector_float4 Y = {0, scale, 0, 0};
    vector_float4 Z = {0, 0, scale, 0};
    vector_float4 W = {0, 0, 0, 1};
    
    matrix_float4x4 mat = {X, Y, Z, W};
    return mat;
};

static matrix_float4x4 matrix_float4x4_rotation(vector_float3 axis, float angle) {
    // euler angles and not quaternions?
//    std::cout << "using my rotation matrix" << std::endl;
    float c = cos(angle);
    float s = sin(angle);
    float t = 1 - c;

    vector_float4 X = { axis.x*axis.x + (1 - axis.x*axis.x)*c, t*axis.x*axis.y - s*axis.z, t*axis.x*axis.z + s*axis.y, 0 };
    vector_float4 Y = { t*axis.x*axis.y + s*axis.z, axis.y*axis.y + (1 - axis.y * axis.y)*c, t*axis.y*axis.z-s*axis.x, 0 };
    vector_float4 Z = { t*axis.z*axis.z - s*axis.y, t*axis.y*axis.z  + s*axis.x, axis.z*axis.z + (1 - axis.z*axis.z)*c, 0 };
    vector_float4 W = { 0, 0, 0, 1};

    matrix_float4x4 mat = {X, Y, Z, W};
    return mat;
};

// it's all column-centric my dude
static matrix_float4x4 matrix_float4x4_perspective(float aspect, float fovy, float near, float far) {
    float fov = 1 / tan(fovy * 0.5);
    float zrange = far - near;
    float zscale = -(far + near)/zrange;
    float wscale = -2*far*near/zrange;
    vector_float4 X = {fov / aspect, 0, 0, 0};
    vector_float4 Y = {0, fov, 0, 0};
    vector_float4 Z = {0, 0, zscale, -1};
    vector_float4 W = {0, 0, wscale, 0};
    
    matrix_float4x4 mat = {X, Y, Z, W};
    return mat;
};

static matrix_float4x4 matrix_orthographic_projection(float left, float right, float top, float bottom)
{
    float near = 0;
    float far = 1;

    float sx = 2 / (right - left);
    float sy = 2 / (top - bottom);
    float sz = 1 / (far - near);
    float tx = (right + left) / (left - right);
    float ty = (top + bottom) / (bottom - top);
    float tz = near / (far - near);

    vector_float4 P = { sx,  0,  0, 0 };
    vector_float4 Q = {  0, sy,  0, 0 };
    vector_float4 R = {  0,  0, sz, 0 };
    vector_float4 S = { tx, ty, tz,  1 };

    matrix_float4x4 mat = { P, Q, R, S };
    return mat;
}

static matrix_float3x3 matrix_float4x4_extract_linear(matrix_float4x4 mv) {
    vector_float3 X = mv.columns[0].xyz;
    vector_float3 Y = mv.columns[1].xyz;
    vector_float3 Z = mv.columns[2].xyz;
    
    matrix_float3x3 n = {X, Y, Z};
    return n;
};

enum EulerAngle {
    yaw,
    pitch,
    roll,
};


// I have to know yaw, roll, pitch
static matrix_float4x4 EulerToMatrix(EulerAngle angleType, float radians){
    vector_float3 yAxis = {0,1,0};
    vector_float3 xAxis = {1,0,0};
    vector_float3 zAxis = {0,0,1};
    
    switch(angleType) {
        case yaw:
            return matrix_float4x4_rotation(yAxis, radians);
        case pitch:
            return matrix_float4x4_rotation(xAxis, radians);
        case roll:
            return matrix_float4x4_rotation(zAxis, radians);
        default:
            // would ideally like to return the identity or something like that
            matrix_float4x4 mat = matrix_float4x4();
            return mat;
    }
}

typedef struct {
    matrix_float4x4 modelViewProjMatrix;
    matrix_float4x4 modelViewMatrix;
    matrix_float3x3 normalMatrix;
    vector_float4 color;
} FrameData;

// picture model
#endif /* AAPLShaderTypes_h */

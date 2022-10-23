/*
See LICENSE folder for this sample’s licensing information.

Abstract:
Metal shaders used for this sample
*/

#include <metal_stdlib>

using namespace metal;

// Include header shared between this Metal shader code and C code executing Metal API commands.
#include "AAPLShaderTypes.h"

struct Light
{
    float3 direction;
    float3 ambientColor;
    float3 diffuseColor;
    float3 specularColor;
};

struct Material
{
    float3 ambientColor;
    float3 diffuseColor;
    float3 specularColor;
    float specularPower;
};

struct Vertex
{
    float4 pos [[position]];
    float4 color;
};

struct Uniforms {
    float4x4 modelViewProjMatrix;
    float4x4 modelViewMatrix;
    float3x3 normalMatrix;
    vector_float4 colors;
};

struct ProjectedVertex {
    float4 position [[position]];
    float3 eye;
    float3 normal;
    float2 vt;
    float4 color;
};

// constants
constant Light light = {
    .direction = { 0.13, 0.72, 0.68 },
    .ambientColor = { 0.05, 0.05, 0.05 },
    .diffuseColor = { 0.9, 0.9, 0.9 },
    .specularColor = { 1, 1, 1 }
};

constant Material material = {
    .ambientColor = { 0.9, 0.1, 0 },
    .diffuseColor = { 0.9, 0.1, 0 },
    .specularColor = { 1, 1, 1 },
    .specularPower = 100
};

//vertex ProjectedVertex vertexShader(uint vid [[vertex_id]], constant MBEVertex *vertices [[buffer(0)]]){
//    ProjectedVertex out;
//
//    out.position = vertices[vid].position;
//    out.color = vertices[vid].color;
//
//    return out;
//}

vertex ProjectedVertex vertexShader(uint vid [[vertex_id]], constant MBEVertex *vertices [[buffer(0)]],
                           constant Uniforms *uniforms [[buffer(1)]],
                           constant vector_uint2 *viewportPtr [[buffer(2)]])
{
    ProjectedVertex out;

    out.position = uniforms->modelViewProjMatrix * vertices[vid].position;
    out.eye = -(uniforms->modelViewMatrix * vertices[vid].position).xyz;
    out.normal = uniforms->normalMatrix * vertices[vid].normal.xyz;
    out.vt = vertices[vid].uv;
//    out.color = vertices[vid].color;
    out.color = uniforms->colors;

    return out;
}


//fragment float4 fragmentShader(ProjectedVertex vin [[stage_in]]){
//    return vin.color;
//}

fragment float4 fragmentShader(ProjectedVertex vin [[stage_in]],
                               texture2d<float> diffuseTexture [[texture(0)]],
                               sampler samplr [[sampler(0)]]) {

//    return vin.color;
//
    if(is_null_texture(diffuseTexture)){
        // can I get here?
        return vin.color;
    }

    float3 diffuseColor = diffuseTexture.sample(samplr, vin.vt).rgb;
    float3 ambientTerm = light.ambientColor * diffuseColor;
    float3 normal = normalize(vin.normal);
    float diffuseIntensity = saturate(dot(normal, light.direction));
    float3 diffuseTerm = diffuseColor * light.diffuseColor * diffuseIntensity;
    float3 specularTerm(0);

    if(diffuseIntensity > 0) {
        float3 eyeDirection = normalize(vin.eye);
        float3 halfway = normalize((light.direction + eyeDirection));
        float specularFactor = pow(saturate(dot(normal, halfway)), material.specularPower);
        specularTerm = light.specularColor * material.specularColor * specularFactor;
    }

//    return float4(1.0, 1.0, 1.0, 1.0);
//    return float4(ambientTerm + diffuseTerm + specularTerm, 1b bb b b nb bb b b  b  )bbbbb
//    return float4(1.0, 0.0, 0., 1);bdm b bdmb bdm b b bb

    return float4(ambientTerm + diffuseTerm + specularTerm, 1.);
}
//vertexShbdfmb b ader(uint vertexID [[vertex_id]],
//            cbm b bdfmq b b bdfmq b nstant AAPLVertex *vertices [[buffer(AAPLVertexInputIndexVertices)]],
//             constant vector_uint2bdmbdfm b bdm bbb bdfm b bdm b b bd   *viewportSizePointer [[buffer(AAPLVertexInputIndexViewportSize)]])
//{
//    RasterizerData out;
//
//    // Index into the array of positions to get the current vertex.
//    // The positions are specified in pixel dimensions (i.e. a value of 100
//    // is 100 pixels from the origin).
//    float2 pixelSpacePosition = vertices[vertexID].position.xy;
//
//    // Get the viewport size and cast to float.
//    vector_float2 viewportSize = vector_float2(*viewportSizePointer);
//
//
//    // To convert from positions in pixel space to positions in clip-space,
//    //  divide the pixel coordinates by half the size of the viewport.
//    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
//    out.position.xy = pixelSpacePosition / (viewportSize / 2.0);
//
//    // Pass the input color directly to the rasterizer.
//    out.color = vertices[vertexID].color;
//
//    return out;
//}

// gonna come up with a new vertext function


//fragment float4 fragmentShader(RasterizerData in [[stage_in]])
//{
//    // Return the interpolated color.
//    return in.color;
//}

// new fragment shader

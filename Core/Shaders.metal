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

struct InstanceData {
    float4 trans; // translation
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

// can I get away with using this for both fragment and vertexShaders?
// for 2D we may just not transform the uniforms all that much (ever?)
vertex ProjectedVertex vertexShader(uint vid [[vertex_id]], uint iid [[instance_id]], constant MBEVertex *vertices [[buffer(0)]],
                           constant Uniforms *uniforms [[buffer(1)]],
                           constant InstanceData *instanceData [[buffer(2)]])
{
    ProjectedVertex out;
    

    out.position = uniforms->modelViewProjMatrix * (vertices[vid].position + instanceData[iid].trans);
    out.eye = -(uniforms->modelViewMatrix * vertices[vid].position).xyz;
    out.normal = uniforms->normalMatrix * vertices[vid].normal.xyz;
    out.vt = vertices[vid].uv;
    
//    out.color = vertices[vid].color; // should almost definitely be this
    out.color = instanceData[iid].color;
    
    return out;
}

// text fragment shader
fragment float4 textFragmentShader(ProjectedVertex vert [[stage_in]],
                               texture2d<float> texture [[texture(0)]],
                                sampler samplr [[sampler(0)]]) {
    
    float4 color = float4(0.1, 0.1, 0.1, 1); // just black for now
    // Outline of glyph is the isocontour with value 50%
//    float edgeDistance = 0.5;
    float edgeDistance = 0.5;
    // Sample the signed-distance field to find distance from this fragment to the glyph outline
    float sampleDistance = texture.sample(samplr, vert.vt).r; // <- is this set up correctly?...
    // Use local automatic gradients to find anti-aliased anisotropic edge width, cf. Gustavson 2012
//    float edgeWidth = 0.75 * length(float2(dfdx(sampleDistance), dfdy(sampleDistance)));
    float edgeWidth = length(float2(dfdx(sampleDistance), dfdy(sampleDistance)));
    // Smooth the glyph edge by interpolating across the boundary in a band with the width determined above
    float insideness = smoothstep(edgeDistance - edgeWidth, edgeDistance + edgeWidth, sampleDistance);
    
    
//    return half4(1, 1, 1, 1.0);
    return float4(color.r, color.g, color.b, insideness);
}



fragment float4 fragmentShader(ProjectedVertex vin [[stage_in]],
                               texture2d<float> diffuseTexture [[texture(0)]],
                               sampler samplr [[sampler(0)]]) {

    return vin.color;

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

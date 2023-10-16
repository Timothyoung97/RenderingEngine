#include "forwardRendering.hlsl"

cbuffer constBufferSSAO : register(b3) {
    float4 kernalSamples[64];
};

Texture2D noiseTexture : register(t5);
SamplerState wrapPointSampler : register(s2);

void ps_ssao(
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
){
    float4 depth = ObjDepthMap.Load(int3(outPosition.xy, 0)); // getting depth from depth buffer
    if (depth.x == 1.f) { // background
        outTarget = float4(.0f, .0f, .0f, .0f);
        return;
    }

    // Convert screen space pos to world space pos
    float x = 2.0f * outPosition.x / viewportDimension.x - 1;
    float y = 1 - (2.0f * outPosition.y / viewportDimension.y);
    float4 clipPos = float4(x, y, depth.x, 1.0f);
    float4 worldPosH = mul(invViewProjection, clipPos); 
    float3 worldPos = worldPosH.xyz / worldPosH.w;

    // create TBN
    float2 noiseScale = float2(viewportDimension.x / 4.0f, viewportDimension.y / 4.0f);

    float3 sampledNormal = ObjNormMap.Load(int3(outPosition.xy, 0)).xyz;
    float3 noiseVec = noiseTexture.Sample(wrapPointSampler, outTexCoord * noiseScale).xyz;

    float3 tangent = normalize(noiseVec - sampledNormal * dot(noiseVec, sampledNormal));
    float3 biTangent = cross(sampledNormal, tangent);
    
    // TBN Matrix
    float4x4 TBNMatrix = {
        float4(tangent,         0.f),
        float4(biTangent,       0.f),
        float4(sampledNormal,   0.f),
        float4(.0f, .0f, .0f,   1.0f)
    };

    // calculate occlusion
    float occlusion = .0f;
    
    [unroll]
    for (int i = 0; i < 64; i++) {
        // find world position of the sampling point
        float3 samplePos = mul(kernalSamples[i], TBNMatrix).xyz;
        samplePos = worldPos + samplePos * .5f;

        // convert to screen space position
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(viewProjection, offset);
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * .5f + .5f;

        // sample depth
        float4 sampleDepth = ObjDepthMap.Load(int3(offset.xy, 0));

        // occlusion contribution
        occlusion += (sampleDepth.x >= samplePos.z + .025f ? 1.f : 0.f);
    }
    
    outTarget = float4(occlusion, .0f, .0f, .0f);
}
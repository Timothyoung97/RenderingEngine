#include "forwardRendering.hlsl"

cbuffer constBufferSSAO : register(b3) {
    float4 kernalSamples[64];
    float sampleRadius;
    float sampleBias;
    float ssaoSwitch;
    float ssaoPad;
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
    float x = 2.0f * outPosition.x / viewportDimension.x - 1.0f;
    float y = 1.0f - (2.0f * outPosition.y / viewportDimension.y);
    float4 clipPos = float4(x, y, depth.x, 1.0f);
    float4 worldPosH = mul(invViewProjection, clipPos); 
    float3 worldPos = worldPosH.xyz / worldPosH.w;

    // create TBN
    float3 sampledNormal = decodeNormal(ObjNormMap.Load(int3(outPosition.xy, 0)).xyz);
    float3x3 TBNMatrix = CalculateTBN(worldPos, sampledNormal, outTexCoord); // all component normalized
    float3 tangentAxis = TBNMatrix[0];
    float3 bitangentAxis = TBNMatrix[1];
    float3 normalAxis = TBNMatrix[2];
    
    float angles[2] = {radians(45.f), radians(-45.f)};

    // calculate occlusion
    float occlusion = .0f;
    
    [unroll]
    for (int i = 0; i < 4; i++) {
        // find world position of the sampling point
        float3 samplePos = worldPos;
        if (i < 2) {
            samplePos += normalize(mul(AngleAxis3x3(angles[i % 2], bitangentAxis), sampledNormal)) * sampleRadius;
        } else {
            samplePos += normalize(mul(AngleAxis3x3(angles[i % 2], normalAxis), sampledNormal)) * sampleRadius;
        }

        // convert to screen space position
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(viewProjection, offset);
        offset.xyz /= offset.w;
        offset.xy = clipToScreenSpace(offset.xy);

        // sample depth
        float4 sampleDepth = ObjDepthMap.Sample(wrapPointSampler, offset.xy);

        // Range check
        float rangeCheck = smoothstep(.0f, 1.f, sampleRadius / (abs(depth.x - sampleDepth.x) * 1000.f)); // 1000.f is dist between near and far plane of camera projection matrix 

        // occlusion contribution
        occlusion += (sampleDepth.x < depth.x + sampleBias ? 1.f : 0.f); //* rangeCheck; // hardcoded bias
    }
    
    outTarget = float4(1 - (occlusion / 4.f), .0f, .0f, .0f);
}

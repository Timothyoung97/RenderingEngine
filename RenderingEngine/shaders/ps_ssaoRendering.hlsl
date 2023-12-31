#include "forwardRendering.hlsl"

cbuffer constBufferSSAO : register(b3) {
    float sampleRadius;
    float3 ssaoPad;
};

SamplerState wrapPointSampler : register(s2);
SamplerState clampPointSampler : register(s3);

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
    float3 sampledNormal = normalize(decodeNormal(ObjNormMap.Load(int3(outPosition.xy, 0)).xyz));
    float3x3 TBNMatrix = CalculateTBN(worldPos, sampledNormal, outTexCoord); // all component normalized
    
    float3 axis[2] = {TBNMatrix[0], TBNMatrix[1]};
    float multipler[2] = {-1.0f, 1.0f};

    // calculate occlusion
    float occlusion = .0f;

    uint ran_state = 1337;
    ran_state = Random(ran_state);
    
    [unroll]
    for (int i = 0; i < 64; i++) {
        // find world position of the sampling point
        float3 samplePos = worldPos;

        // random rotate to form a hemisphere
        int axisSelector = int(Random01(ran_state) * 11) % 2;
        float3 firstRotate = rodriguesRotate(sampledNormal, axis[axisSelector], (Random01(ran_state) * 2 - 1.0f) * 90.f);
        samplePos += rodriguesRotate(firstRotate, axis[axisSelector ^ 1], (Random01(ran_state) * 2 - 1.0f) * 90.f) * sampleRadius * Random01(ran_state);
        // samplePos += float3(Random01(ran_state) * 2.0f - 1.0f, Random01(ran_state) * 2.0f - 1.0f, Random01(ran_state) * 2.0f - 1.0f) * sampleRadius; // debug line

        // convert to screen space position
        float4 offset = float4(samplePos, 1.0f);
        offset = mul(viewProjection, offset);
        offset.xyz /= offset.w;

        float2 sampleClipXY = offset.xy;

        offset.xy = clipToScreenSpace(offset.xy);

        offset.z = offset.z * .5f + .5f;

        // sample depth
        float4 sampleDepth = ObjDepthMap.Sample(clampPointSampler, offset.xy);
        float4 sampleClipPos = float4(sampleClipXY, sampleDepth.x, 1.0f);
        float4 sampleWorldPosH = mul(invViewProjection, sampleClipPos); 
        float3 sampleWorldPos = sampleWorldPosH.xyz / sampleWorldPosH.w;

        // Range check
        float rangeCheck = smoothstep(.0f, 1.f, sampleRadius / length(worldPos - sampleWorldPos));
        // float rangeCheck = length(worldPos - sampleWorldPos) < sampleRadius ? 1 : 0; // debug line

        // occlusion contribution
        //occlusion += (length(sampleWorldPos - camPos.xyz) < length(worldPos - camPos.xyz) ? 1.f : 0.f) * rangeCheck; // debug line
        occlusion += (sampleDepth.x < depth.x ? 1.f : 0.f) * rangeCheck; // hardcoded bias
    }
    
    outTarget = float4(1 - (occlusion / 64.f), .0f, .0f, .0f);
}

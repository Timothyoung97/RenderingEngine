#include "forwardRendering.hlsl"

cbuffer constBufferSSAO : register(b3) {
    float4 kernalSamples[64];
};

Texture2D ObjDepthMap : register(t5);
SamplerState wrapPointSampler : register(s2);

const vec2 noiseScale = float2(viewportDimension.x / 4.0f, viewportDimension.y / 4.0f);

void ps_ssao(
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
){
    
}
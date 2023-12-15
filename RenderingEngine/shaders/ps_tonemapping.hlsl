#include "utility_tonemappers.hlsl"

Texture2D hdrTexture : register(t0);
Buffer<float> luminAvg : register(t1);
Texture2D bloomTexture : register(t2);

cbuffer constBufferHDR : register(b0) {
    float middleGrey;
    float3 pad_hdr;
};

void ps_tonemap(
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
){

    const float gamma = 2.2f;

    float4 currColor = hdrTexture.Load(int3(outPosition.xy, 0));
    float4 bloomColor = bloomTexture.Load(int3(outPosition.xy, 0));
    float4 finalColor = lerp(currColor, bloomColor, .3f);

    float fLumScale = middleGrey / luminAvg[0];

    float4 tonemappedRes = float4(ACESFilm(finalColor.xyz * fLumScale), 0);

    tonemappedRes = pow(tonemappedRes, float(1.0f / gamma));

    outTarget = tonemappedRes;
}

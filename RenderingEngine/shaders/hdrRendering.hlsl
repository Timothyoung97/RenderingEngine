#include "tonemappers.hlsl"

Texture2D hdrTexture : register(t8);

cbuffer constBufferHDR : register(b4) {
    float exposure;
    float3 pad_hdr;
};

void ps_hdr_tonedown(
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
){

    const float gamma = 2.2f;

    float4 currColor = hdrTexture.Load(int3(outPosition.xy, 0));

    float4 tonemappedRes = float4(ACESFilm(currColor.xyz * exposure), 0);

    tonemappedRes = pow(tonemappedRes, float(1.0f / gamma));

    outTarget = tonemappedRes;
}

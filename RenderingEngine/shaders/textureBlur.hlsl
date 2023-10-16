#include "helper.hlsl"
#include "forwardRendering.hlsl"

Texture2D targetTexture : register(t6);

void ps_texture_blur(
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
) {
    float2 texelSize = float2(1.f / viewportDimension.x, 1.f / viewportDimension.y);

    float4 currPixelColor = targetTexture.Sample(ObjSamplerStateLinear, outTexCoord);
    
    [unroll]
    for (int x = -1; x <= 1; ++x) {
        [unroll]
        for (int y = -1; y <= 1; ++y) {
            float4 samplePixelColor = targetTexture.Sample(ObjSamplerStateLinear, outTexCoord.xy + float2(x, y) * texelSize);
            currPixelColor.x += samplePixelColor.x;
        }
    }

    outTarget = float4(currPixelColor.x / 9.f, .0f, .0f, .0f);    
}
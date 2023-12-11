struct BloomConfig {
    uint2 srcViewportDimension;
    uint2 destViewportDimension;
    uint readIdx;
    float3 pad;
};

cbuffer constBufferBloomConfig : register (b0) {
    BloomConfig bloomConfig;
};

RWTexture2DArray<float4> upsampleTextures : register(u0);

float3 upsample(uint2 texelLocation, uint readIndex, float radius) {
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = upsampleTextures[int3(texelLocation.x - radius,  texelLocation.y + radius,   readIdx)];
    float3 b = upsampleTextures[int3(texelLocation.x,           texelLocation.y + radius,   readIdx)];
    float3 c = upsampleTextures[int3(texelLocation.x + radius,  texelLocation.y + radius,   readIdx)];

    float3 d = upsampleTextures[int3(texelLocation.x - radius,  texelLocation.y,            readIdx)];
    float3 e = upsampleTextures[int3(texelLocation.x,           texelLocation.y,            readIdx)];
    float3 f = upsampleTextures[int3(texelLocation.x + radius,  texelLocation.y,            readIdx)];

    float3 g = upsampleTextures[int3(texelLocation.x - radius,  texelLocation.y - radius,   readIdx)];
    float3 h = upsampleTextures[int3(texelLocation.x,           texelLocation.y - radius,   readIdx)];
    float3 i = upsampleTextures[int3(texelLocation.x + radius,  texelLocation.y - radius,   readIdx)];

    float3 res = e * 4.f;
    res += (b + d + f + h) * 2.f;
    res += 

    return res;
}

[numthreads(16, 16, 1)]
void cs_bloomUpsample (
    uint3 dispatchThreadID : SV_DispatchThreadID
) {
    if (dispatchThreadID.x > bloomConfig.destViewportDimension.x || dispatchThreadID.y > bloomConfig.destViewportDimension.y ) {
        return;
    }


    


}
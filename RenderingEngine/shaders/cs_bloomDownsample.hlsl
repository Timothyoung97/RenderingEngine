struct BloomConfig {
    uint2 srcViewportDimension;
    uint2 destViewportDimension;
    uint readIdx;
    float3 pad;
};

cbuffer constBufferBloomConfig : register (b0) {
    BloomConfig bloomConfig;
};

RWTexture2DArray<float4> downsampleTextures : register(u0);

//bi-linear filtering
float3 downsample(uint2 texelLocation, uint readIndex) {
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = downsampleTextures[int3(texelLocation.x - 2,     texelLocation.y + 2,    readIndex)].xyz;
    float3 b = downsampleTextures[int3(texelLocation.x,         texelLocation.y + 2,    readIndex)].xyz;
    float3 c = downsampleTextures[int3(texelLocation.x + 2,     texelLocation.y + 2,    readIndex)].xyz;

    float3 d = downsampleTextures[int3(texelLocation.x - 2,     texelLocation.y,        readIndex)].xyz;  
    float3 e = downsampleTextures[int3(texelLocation.x,         texelLocation.y,        readIndex)].xyz;
    float3 f = downsampleTextures[int3(texelLocation.x + 2,     texelLocation.y,        readIndex)].xyz;

    float3 g = downsampleTextures[int3(texelLocation.x - 2,     texelLocation.y - 2,    readIndex)].xyz;
    float3 h = downsampleTextures[int3(texelLocation.x,         texelLocation.y - 2,    readIndex)].xyz;
    float3 i = downsampleTextures[int3(texelLocation.x + 2,     texelLocation.y - 2,    readIndex)].xyz;

    float3 j = downsampleTextures[int3(texelLocation.x - 1,     texelLocation.y + 1,    readIndex)].xyz;
    float3 k = downsampleTextures[int3(texelLocation.x + 1,     texelLocation.y + 1,    readIndex)].xyz;
    float3 l = downsampleTextures[int3(texelLocation.x - 1,     texelLocation.y - 1,    readIndex)].xyz;
    float3 m = downsampleTextures[int3(texelLocation.x + 1,     texelLocation.y - 1,    readIndex)].xyz;

    float3 res = e * .125f;
    res += (a + c + g + i) * .03125f;
    res += (b + d + f + h) * .0625f;
    res += (j + k + l + m) * .125f;

    return res;
}

[numthreads(16, 16, 1)]
void cs_bloomDownsample (
    uint3 dispatchThreadID : SV_DispatchThreadID
) {
    if (dispatchThreadID.x > bloomConfig.destViewportDimension.x || dispatchThreadID.y > bloomConfig.destViewportDimension.y ) {
        return;
    }

    float3 downsampledRes = downsample(dispatchThreadID.xy * 2, bloomConfig.readIdx);

    downsampleTextures[int3(dispatchThreadID.xy, bloomConfig.readIdx ? 0 : 1)] = float4(downsampledRes, 1.f);
}
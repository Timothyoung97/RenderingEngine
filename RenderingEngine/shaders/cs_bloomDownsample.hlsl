#define RGB_TO_LUM float3(0.2125, .07154, 0.0721)

struct BloomConfig {
    uint2 srcViewportDimension;
    uint2 destViewportDimension;
    float sampleRadius;
    float3 pad;
};

cbuffer constBufferBloomConfig : register (b0) {
    BloomConfig bloomConfig;
};

Texture2D<float3> sampleTexture : register(t0);
SamplerState ssMinMagMipLinearClamp : register(s0);
Buffer<float> luminAverage : register(t1);

RWTexture2D<float3> downsampleTextures : register(u0);

float3 sample(float2 texCoord, float2 viewDimensionUVExtent) {
    texCoord.x = clamp(texCoord.x, 0, viewDimensionUVExtent.x);    
    texCoord.y = clamp(texCoord.y, 0, viewDimensionUVExtent.y);    
    return sampleTexture.SampleLevel(ssMinMagMipLinearClamp, texCoord, 0).xyz;
}

//bi-linear filtering
float3 downsample(float2 texcoord, float2 textureCoordOffset, float2 sampleViewDimension) {
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = sample(float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y + 2 * textureCoordOffset.y), sampleViewDimension);
    float3 b = sample(float2(texcoord.x,                                texcoord.y + 2 * textureCoordOffset.y), sampleViewDimension);
    float3 c = sample(float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y + 2 * textureCoordOffset.y), sampleViewDimension);
    float3 d = sample(float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y),                            sampleViewDimension);
    float3 e = sample(float2(texcoord.x,                                texcoord.y),                            sampleViewDimension);
    float3 f = sample(float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y),                            sampleViewDimension);
    float3 g = sample(float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y - 2 * textureCoordOffset.y), sampleViewDimension);
    float3 h = sample(float2(texcoord.x,                                texcoord.y - 2 * textureCoordOffset.y), sampleViewDimension);
    float3 i = sample(float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y - 2 * textureCoordOffset.y), sampleViewDimension);
    float3 j = sample(float2(texcoord.x - textureCoordOffset.x,         texcoord.y + textureCoordOffset.y),     sampleViewDimension);
    float3 k = sample(float2(texcoord.x + textureCoordOffset.x,         texcoord.y + textureCoordOffset.y),     sampleViewDimension);
    float3 l = sample(float2(texcoord.x - textureCoordOffset.x,         texcoord.y - textureCoordOffset.y),     sampleViewDimension);
    float3 m = sample(float2(texcoord.x + textureCoordOffset.x,         texcoord.y - textureCoordOffset.y),     sampleViewDimension);

    float3 res = e * .125f;
    res += (a + c + g + i) * .03125f;
    res += (b + d + f + h) * .0625f;
    res += (j + k + l + m) * .125f;

    return dot(res, RGB_TO_LUM) > luminAverage[0] ? res : 0;
}

[numthreads(8, 8, 1)]
void cs_bloomDownsample (
    uint3 dispatchThreadID : SV_DispatchThreadID
) {
    if (dispatchThreadID.x >= bloomConfig.destViewportDimension.x || dispatchThreadID.y >= bloomConfig.destViewportDimension.y ) {
        return;
    }

    float2 invSrcViewportDimension = 1.f / (bloomConfig.srcViewportDimension);

    // multiply by 2 to restore to sample texture size, then multiply by invViewDimension to get texcoord
    float2 texcoord = (dispatchThreadID.xy + float2(.5f, .5f)) * 2.0f * invSrcViewportDimension; 

    float3 downsampledRes = downsample(texcoord, invSrcViewportDimension, bloomConfig.destViewportDimension * 2.f * invSrcViewportDimension - invSrcViewportDimension);

    downsampleTextures[dispatchThreadID.xy] = downsampledRes;
}
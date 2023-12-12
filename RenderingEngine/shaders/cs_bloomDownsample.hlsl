struct BloomConfig {
    uint2 srcViewportDimension;
    uint2 destViewportDimension;
    float2 invSrcViewportDimension;
    float2 invDestViewportDimension;
    float sampleRadius;
    float3 pad;
};

cbuffer constBufferBloomConfig : register (b0) {
    BloomConfig bloomConfig;
};

Texture2D<float3> sampleTexture : register(t0);
SamplerState ssMinMagMipLinearClamp : register(s0);

RWTexture2D<float3> downsampleTextures : register(u0);

//bi-linear filtering
float3 downsample(float2 texcoord, float2 textureCoordOffset) {
    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y + 2 * textureCoordOffset.y), 0).xyz;
    float3 b = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,                                texcoord.y + 2 * textureCoordOffset.y), 0).xyz;
    float3 c = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y + 2 * textureCoordOffset.y), 0).xyz;

    float3 d = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y),                            0).xyz;
    float3 e = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,                                texcoord.y),                            0).xyz;
    float3 f = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y),                            0).xyz;

    float3 g = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - 2 * textureCoordOffset.x,     texcoord.y - 2 * textureCoordOffset.y), 0).xyz;
    float3 h = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,                                texcoord.y - 2 * textureCoordOffset.y), 0).xyz;
    float3 i = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + 2 * textureCoordOffset.x,     texcoord.y - 2 * textureCoordOffset.y), 0).xyz;

    float3 j = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - textureCoordOffset.x,         texcoord.y + textureCoordOffset.y),     0).xyz;
    float3 k = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + textureCoordOffset.x,         texcoord.y + textureCoordOffset.y),     0).xyz;
    float3 l = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - textureCoordOffset.x,         texcoord.y - textureCoordOffset.y),     0).xyz;
    float3 m = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + textureCoordOffset.x,         texcoord.y - textureCoordOffset.y),     0).xyz;

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

    // multiply by 2 to restore to sample texture size, then multiply by invViewDimension to get texcoord
    float2 texcoord = dispatchThreadID.xy * 2.0f * bloomConfig.invSrcViewportDimension; 

    float3 downsampledRes = downsample(texcoord, bloomConfig.invSrcViewportDimension);

    downsampleTextures[dispatchThreadID.xy] = downsampledRes;
}
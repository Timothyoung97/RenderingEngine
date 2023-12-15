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

RWTexture2D<float3> upsampleTexture : register(u0);

float3 sample(float2 texCoord, float2 viewDimensionUVExtent) {
    texCoord.x = clamp(texCoord.x, 0, viewDimensionUVExtent.x);    
    texCoord.y = clamp(texCoord.y, 0, viewDimensionUVExtent.y);    
    return sampleTexture.SampleLevel(ssMinMagMipLinearClamp, texCoord, 0).xyz;
}

float3 upsample(float2 texcoord, float radius, float2 sampleViewDimension) {
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = sample(float2(texcoord.x - radius,  texcoord.y + radius), sampleViewDimension);
    float3 b = sample(float2(texcoord.x,           texcoord.y + radius), sampleViewDimension);
    float3 c = sample(float2(texcoord.x + radius,  texcoord.y + radius), sampleViewDimension);

    float3 d = sample(float2(texcoord.x - radius,  texcoord.y         ), sampleViewDimension);
    float3 e = sample(float2(texcoord.x,           texcoord.y         ), sampleViewDimension);
    float3 f = sample(float2(texcoord.x + radius,  texcoord.y         ), sampleViewDimension);

    float3 g = sample(float2(texcoord.x - radius,  texcoord.y - radius), sampleViewDimension);
    float3 h = sample(float2(texcoord.x,           texcoord.y - radius), sampleViewDimension);
    float3 i = sample(float2(texcoord.x + radius,  texcoord.y - radius), sampleViewDimension);

    float3 res = e * 4.f;
    res += (b + d + f + h) * 2.f;
    res += (a + c + g + i);
    res *= 1.f / 16.f;

    return res;
}

[numthreads(8, 8, 1)]
void cs_bloomUpsample (
    uint3 dispatchThreadID : SV_DispatchThreadID
) {
    if (dispatchThreadID.x >= bloomConfig.destViewportDimension.x || dispatchThreadID.y >= bloomConfig.destViewportDimension.y ) {
        return;
    }

    float2 invSrcViewportDimension = 1.f / bloomConfig.srcViewportDimension;

    // sampling a 1 level lower mips, then obtain the texcoord
    float2 texcoord = (dispatchThreadID.xy + float2(.5f, .5f)) * .5f * invSrcViewportDimension; 

    float3 upsampledRes = upsample(texcoord, bloomConfig.sampleRadius, bloomConfig.destViewportDimension * .5f * invSrcViewportDimension - invSrcViewportDimension);

    upsampleTexture[dispatchThreadID.xy] = upsampledRes;
    
}
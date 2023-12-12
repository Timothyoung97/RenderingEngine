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

float3 upsample(float2 texcoord, float radius) {
    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    float3 a = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - radius,  texcoord.y + radius), 0).xyz;
    float3 b = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,           texcoord.y + radius), 0).xyz;
    float3 c = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + radius,  texcoord.y + radius), 0).xyz;

    float3 d = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - radius,  texcoord.y         ), 0).xyz;
    float3 e = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,           texcoord.y         ), 0).xyz;
    float3 f = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + radius,  texcoord.y         ), 0).xyz;

    float3 g = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x - radius,  texcoord.y - radius), 0).xyz;
    float3 h = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x,           texcoord.y - radius), 0).xyz;
    float3 i = sampleTexture.SampleLevel(ssMinMagMipLinearClamp, float2(texcoord.x + radius,  texcoord.y - radius), 0).xyz;

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
    if (dispatchThreadID.x > bloomConfig.destViewportDimension.x || dispatchThreadID.y > bloomConfig.destViewportDimension.y ) {
        return;
    }

    float2 invSrcViewportDimension = 1.f / bloomConfig.srcViewportDimension;

    // sampling a 1 level lower mips, then obtain the texcoord
    float2 texcoord = (dispatchThreadID.xy * .5f + float2(.5f, .5f)) * invSrcViewportDimension; 

    float3 upsampledRes = upsample(texcoord, bloomConfig.sampleRadius);

    upsampleTexture[dispatchThreadID.xy] = upsampledRes;
    
}
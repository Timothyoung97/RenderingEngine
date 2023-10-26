#define EPSILON 0.005
#define RGB_TO_LUM float3(0.2125, .07154, 0.0721)

struct LuminanceConfig {
    float2 luminance; // x = min, y = max
    float timeCoeff;
    int numPixels;
    uint2 viewportDimension;
    uint2 pad;
};

cbuffer constBufferLuminConfig : register(b0) {
    LuminanceConfig luminConfig;
};

Texture2D hdrTexture : register(t8); // 1920 x 1080
RWBuffer<uint> histogram : register(u0); // 256 buckets: 0 ~ 255 

groupshared uint localGroupHistogram[256];

uint colorToBucket(float3 hdrColor, float minLogLum, float inverseLogLumRange) {

    float lum = dot(hdrColor, RGB_TO_LUM);

    if (lum < EPSILON) {
        return 0;
    }

    float logLum = clamp((log2(lum) - minLogLum) * inverseLogLumRange, .0f, 1.f);

    return uint(logLum * 254.f + 1.f);
}

[numthreads(16, 16, 1)]
void cs_genHistogram(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint localThreadIndex : SV_GroupIndex
) {

    localGroupHistogram[localThreadIndex] = 0;

    GroupMemoryBarrierWithGroupSync();

    if (dispatchThreadID.x < luminConfig.viewportDimension.x || dispatchThreadID.y < luminConfig.viewportDimension.y) {

        float4 hdrColor = hdrTexture.Load(int3(dispatchThreadID.xy, 0));
        uint bucketIdx = colorToBucket(hdrColor.xyz, log2(luminConfig.luminance.x), 1.f / (log2(luminConfig.luminance.y) - log2(luminConfig.luminance.x)));
        InterlockedAdd(localGroupHistogram[bucketIdx], 1);
    }

    GroupMemoryBarrierWithGroupSync();

    InterlockedAdd(histogram[localThreadIndex], localGroupHistogram[localThreadIndex]);    
}
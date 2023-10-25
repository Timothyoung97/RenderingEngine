struct LuminanceConfig {
    float2 luminance; // x = min, y = max
    float timeCoeff;
    float numPixels;
    uint2 viewportDimension;
};

cbuffer constBufferLuminConfig : register(b0) {
    LuminanceConfig luminConfig;
};

RWBuffer<uint> histogram : register(u0); // 256 buckets: 0 ~ 255 
RWBuffer<float> luminAvg : register(u1); // buffer of size 1, containing exposure value

groupshared uint localGroupHistogram[256];

[numthreads(256, 1, 1)]
void cs_luminAverage(
    uint3 dispatchThreadID : SV_DispatchThreadID,
    uint localThreadIndex : SV_GroupIndex
){
    uint bucketValue = histogram[localThreadIndex];
    localGroupHistogram[localThreadIndex] = bucketValue * localThreadIndex;

    GroupMemoryBarrierWithGroupSync();

    histogram[localThreadIndex] = 0;

    [unroll]
    for (uint cutoff = (256 >> 1); cutoff > 0; cutoff >>= 1) {
        if (localThreadIndex < cutoff) {
            localGroupHistogram[localThreadIndex] += localGroupHistogram[localThreadIndex + cutoff];
        }
        GroupMemoryBarrierWithGroupSync();
    }

    if (dispatchThreadID.x == 0) {
        float weightedLogAverage = (localGroupHistogram[0] / max(luminConfig.numPixels - float(bucketValue), 1.0f)) - 1.0f;

        float weightedAvgLum = exp2(((weightedLogAverage / 254.f) * log2(luminConfig.luminance.y - luminConfig.luminance.x)) + log2(luminConfig.luminance.x));

        float lumLastFrame = luminAvg[0];
        
        float adaptedLum = lumLastFrame + (weightedAvgLum - lumLastFrame) * luminConfig.timeCoeff;

        luminAvg[0] = adaptedLum;
    }
}
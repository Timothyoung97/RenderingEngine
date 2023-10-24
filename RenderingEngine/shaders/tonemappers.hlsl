//--------------------------------------------------------------------------------------
// Reinhard
//--------------------------------------------------------------------------------------
float3 Reinhard(float3 color)
{
    return color / (1 + color);
}

//--------------------------------------------------------------------------------------
// Hable's filmic
//--------------------------------------------------------------------------------------
float3 Uncharted2TonemapOp(float3 x)
{
    float A = 0.15f;
    float B = 0.50f;
    float C = 0.10f;
    float D = 0.20f;
    float E = 0.02f;
    float F = 0.30f;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 Uncharted2Tonemap(float3 color)
{
    float W = 11.2;
    return Uncharted2TonemapOp(2.0 * color) / Uncharted2TonemapOp(W);
}

//--------------------------------------------------------------------------------------
// https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
//--------------------------------------------------------------------------------------
float3 ACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

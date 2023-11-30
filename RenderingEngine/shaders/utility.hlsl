static float2 clipToScreenSpace(float2 xy) {
    return xy * float2(0.5f, -0.5f) + float2(0.5f, 0.5f);
}

static float2 screenToClipSpace(float2 xy, float2 viewportDimension) {
    return (xy / viewportDimension) * float2(2.0f, -2.0) + float2(-1, 1);
}

static float3 encodeNormal(float3 srcNormal)
{
    return srcNormal * .5f + .5f;
}

static float3 decodeNormal(float3 srcNormal)
{
    return srcNormal * 2.0f - 1.0f;
}

static float3x3 CalculateTBN(float3 p, float3 n, float2 tex) {
    float3 dp1 = ddx(p);
    float3 dp2 = ddy(p);
    float2 duv1 = ddx(tex);
    float2 duv2 = ddy(tex);

    float3x3 M = float3x3(dp1, dp2, cross(dp1, dp2));
    float2x3 inverseM = float2x3(cross(M[1], M[2]), cross(M[2], M[0]));
    float3 t = normalize(mul(float2(duv1.x, duv2.x), inverseM));
    float3 b = normalize(mul(float2(duv1.y, duv2.y), inverseM));
    return float3x3(t, b, n);
}

// Rotation with angle (in radians) and axis
float3x3 AngleAxis3x3(float angle, float3 axis)
{
    float c, s;
    sincos(angle, s, c);

    float t = 1 - c;
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;

    return float3x3(
        t * x * x + c,      t * x * y - s * z,  t * x * z + s * y,
        t * x * y + s * z,  t * y * y + c,      t * y * z - s * x,
        t * x * z - s * y,  t * y * z + s * x,  t * z * z + c
    );
}

float3 rodriguesRotate (float3 v, float3 n, float a) {
	return v * cos(a) + cross(n, v) * sin(a) + n * dot(n, v) * (1. - cos(a));
}

float nrand(float2 uv) {
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

bool isNaN(float x) {
    return (asuint(x) & 0x7fffffff) > 0x7f800000;
}

// Generate a random 32-bit integer
uint Random(inout uint state)
{
    // Xorshift algorithm from George Marsaglia's paper.
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state << 5);
    return state;
}

// Generate a random float in the range [0.0f, 1.0f)
float Random01(inout uint state)
{
    return asfloat(0x3f800000 | Random(state) >> 9) - 1.0;
}
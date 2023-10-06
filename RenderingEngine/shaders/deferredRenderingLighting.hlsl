struct Light {
    float3 dir;
    float4 ambient;
    float4 diffuse;
};

struct PointLight {
    float3 dir;
    float pad;
    float3 pos;
    float range;
    float3 att;
    float pad2;
    float4 ambient;
    float4 diffuse;
};

// Global 
cbuffer constBuffer : register(b0) {
    float4 camPos;
    matrix viewProjection;
    matrix lightviewProjection[4];
    float4 planeIntervals;
    Light dirLight;
    int numPtLights;
    float2 shadowMapDimension;
    int csmDebugSwitch;
};

// Per Object
cbuffer constBuffer2 : register(b1) {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
    uint hasNormMap;
};

Texture2D ObjTexture : register(t0);
Texture2D ObjNormMap : register(t1);
StructuredBuffer<PointLight> pointLights : register(t2);
Texture2D ObjShadowMap : register(t3);
Texture2D ObjDepthMap : register(t4);

SamplerState ObjSamplerStateLinear : register(s0);
SamplerComparisonState ObjSamplerStateMipPtWhiteBorder : register(s1);

void ps_lightingPass (
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
) {

    float x = 2.0f * outPosition.x / 1920.f  - 1;
    float y = 1 - (2.0f * outPosition.y / 1080.f);

    // getting depth from depth buffer
    float depth = ObjDepthMap.Load(int3(outPosition.xy, 0));

    float4 clipPos = float4(x, y, depth, 1.0f);

    matrix invViewProjection = inverse(viewProjection); // pass to const buffer

    float4 worldPosH = mul(invViewProjection, clipPos); 
    float3 worldPos = worldPosH.xyz / worldPosH.w;


}
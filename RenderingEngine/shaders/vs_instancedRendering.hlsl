#include "globalResources.hlsl"

// Global 
cbuffer constBuffer : register(b0) {
    matrix viewProjection;
};

// Batch offset
cbuffer constBuffer2 : register(b1) {
    uint offset;
};

struct InstanceInfo {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
    uint hasNormMap;
    uint2 pad;
};

StructuredBuffer<InstanceInfo> instanceInfoQueue : register(t0);

// Vertex Shader
void vs_instancedRendering (
    in float3 inPosition : POSITION,
    in float3 inNormal : NORMAL,
    in float3 inTangent : TANGENT,
    in float2 inTexCoord : TEXCOORD,
    out float4 outPosition : SV_POSITION,
    out float4 outWorldPosition : TEXCOORD0,
    out float4 outNormal : TEXCOORD1,
    out float4 outTangent : TEXCOORD2,
    out float2 outTexCoord : TEXCOORD3,
    out float4 outColor : TEXCOORD4,
    out float4 outTextuerInfo : TEXCOORD5,
    uint instanceID : SV_InstanceID
) 
{   

    InstanceInfo iInfo = instanceInfoQueue[offset + instanceID];

    // Position
    float4 localPos = float4(inPosition, 1);
    float4 worldPos = mul(iInfo.transformation, localPos);
    outPosition = mul(viewProjection, worldPos);

    // world position
    outWorldPosition = worldPos;

    // Normal
    float4 tempInNormal = float4(inNormal, 0);
    outNormal = normalize(mul(iInfo.normalMatrix, tempInNormal));

    // Tangent
    float4 tempInTangent = float4(inTangent, 0);
    outTangent = normalize(mul(iInfo.normalMatrix, tempInTangent)); // using normal matrix to move tangent

    // Texture
    outTexCoord = inTexCoord;

    // Color 
    outColor = iInfo.color;

    // Texture Info
    outTextuerInfo = float4(iInfo.isWithTexture, iInfo.hasNormMap, .0f, .0f);
};
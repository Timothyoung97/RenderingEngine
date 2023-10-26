
cbuffer constBuffer : register(b0) {
    matrix lightviewProjection;
}

// Per Object
cbuffer constBuffer2 : register(b1) {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
    uint hasNormMap;
};

// Vertex Shader
void vs_shadowCast (
    in float3 inPosition : POSITION,
    in float3 inNormal : NORMAL,
    in float3 inTangent : TANGENT,
    in float2 inTexCoord : TEXCOORD,
    out float4 outPosition : SV_POSITION,
    out float4 outWorldPosition : TEXCOORD0,
    out float4 outNormal : TEXCOORD1,
    out float4 outTangent : TEXCOORD2,
    out float2 outTexCoord : TEXCOORD3
) 
{   
    // Position
    float4 localPos = float4(inPosition, 1);
    float4 worldPos = mul(transformation, localPos);
    outPosition = mul(lightviewProjection, worldPos);

    // world position
    outWorldPosition = worldPos;

    // Normal
    float4 tempInNormal = float4(inNormal, 0);
    outNormal = normalize(mul(normalMatrix, tempInNormal));

    // Tangent
    float4 tempInTangent = float4(inTangent, 0);
    outTangent = normalize(mul(normalMatrix, tempInTangent)); // using normal matrix to move tangent

    // Texture
    outTexCoord = inTexCoord;
};
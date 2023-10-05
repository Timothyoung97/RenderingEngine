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

SamplerState ObjSamplerStateLinear : register(s0);

// Pixel Shader for albedo
void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTargetAlbedo: SV_TARGET0,
    out float4 outTargetNormal: SV_TARGET1
) {

    // Albedo
    outTargetAlbedo = color;

    if (isWithTexture) {
        outTargetAlbedo = ObjTexture.Sample(ObjSamplerStateLinear, vOutTexCoord);
    }

    // Normal
    outTargetNormal = float4(.0f, .0f, .0f, .0f);

    if (hasNormMap) {
        float4 normalMap = ObjNormMap.Sample(ObjSamplerStateLinear, vOutTexCoord);

        normalMap = (2.0f * normalMap) - 1.0f; // change from [0, 1] to [-1, 1]
        
        // Bitangent TODO: Should be cross(N, T)
        float3 biTangent = normalize(-1.0f * cross(vOutNormal.xyz, vOutTangent.xyz)); // create biTangent

        // TBN Matrix
        float4x4 texSpace = {
            vOutTangent,
            float4(biTangent, 0),
            vOutNormal,
            float4(.0f, .0f, .0f, 1.0f)
        };

        outTargetNormal = normalize(mul(normalMap, texSpace)); // convert normal from normal map to texture space
    }
}

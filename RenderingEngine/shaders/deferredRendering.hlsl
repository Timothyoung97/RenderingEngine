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

// 1st deferred draw: Draw for normal and opaque
void ps_deferred_gbuffer (
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
    outTargetNormal = normalize(vOutNormal);

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
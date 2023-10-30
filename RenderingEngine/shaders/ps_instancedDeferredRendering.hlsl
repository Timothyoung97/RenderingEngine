#include "utility.hlsl"

SamplerState ObjSamplerStateLinear : register(s0);
SamplerComparisonState ObjSamplerStateMipPtWhiteBorder : register(s1);

Texture2D ObjTexture : register(t0);
Texture2D ObjNormMap : register(t1);

// 1st deferred draw: Draw for normal and opaque
void ps_instanced_deferred_gbuffer (
    in float4 outPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 outNormal : TEXCOORD1,
    in float4 outTangent : TEXCOORD2,
    in float2 outTexCoord : TEXCOORD3,
    in float4 outColor : TEXCOORD4,
    in float4 outTextuerInfo : TEXCOORD5,
    out float4 outTargetAlbedo: SV_TARGET0,
    out float3 outTargetNormal: SV_TARGET1
) {

    outTargetAlbedo = outColor;
    if (outTextuerInfo.x) {  // check if have diffuse texture
        outTargetAlbedo = ObjTexture.Sample(ObjSamplerStateLinear, outTexCoord);
    }

    
    outTargetNormal = encodeNormal(normalize(outNormal).xyz);
    if (outTextuerInfo.y) { // check if have normal texture
        float3 normalMap = decodeNormal(ObjNormMap.Sample(ObjSamplerStateLinear, outTexCoord).xyz); // change from [0, 1] to [-1, 1]

        // Bitangent TODO: Should be cross(N, T)
        float3 biTangent = normalize(-1.0f * cross(outNormal.xyz, outTangent.xyz)); // create biTangent

        // TBN Matrix
        float4x4 texSpace = {
            outTangent,
            float4(biTangent, 0),
            outNormal,
            float4(.0f, .0f, .0f, 1.0f)
        };

        outTargetNormal = encodeNormal(normalize(mul(float4(normalMap, .0f), texSpace)).xyz); // convert normal from normal map to texture space
    }
}
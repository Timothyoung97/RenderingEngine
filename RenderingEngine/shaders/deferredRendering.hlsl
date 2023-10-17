#include "forwardRendering.hlsl"
#include "helper.hlsl"

// 1st deferred draw: Draw for normal and opaque
void ps_deferred_gbuffer (
    in float4 vOutPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTargetAlbedo: SV_TARGET0,
    out float3 outTargetNormal: SV_TARGET1
) {

    // Albedo
    outTargetAlbedo = sampleTexture(vOutTexCoord);

    // Normal
    outTargetNormal = encodeNormal(sampleNormal(vOutTexCoord, vOutNormal, vOutTangent).xyz);
}
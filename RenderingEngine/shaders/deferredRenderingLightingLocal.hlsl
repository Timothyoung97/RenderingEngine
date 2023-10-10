#include "forwardRendering.hlsl"

cbuffer constBufferLight : register(b3) {
    uint currLightIdx;
}

void ps_lightingLocalPass (
    in float4 vOutPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTarget: SV_TARGET
) {

    float4 sampledAlbedo = ObjTexture.Load(int3(vOutPosition.xy, 0));
    float3 sampledNormal = ObjNormMap.Load(int3(vOutPosition.xy, 0)).xyz;

    // vector between light pos and pixel pos
    float3 pixelToLightV = pointLights[currLightIdx].pos - outWorldPosition.xyz;
    float d = length(pixelToLightV);   

    float3 localLight = float3(.0f, .0f, .0f);

    if (d <= pointLights[currLightIdx].range) {
        pixelToLightV = pixelToLightV / d; // convert pixelToLightV to an unit vector
        float cosAngle = dot(pixelToLightV, sampledNormal.xyz); // find the cos(angle) between light and normal

        if (cosAngle > 0.0f) {
            localLight = cosAngle * sampledAlbedo.xyz * pointLights[currLightIdx].diffuse.xyz; // add light to finalColor of pixel
            localLight = localLight / (pointLights[currLightIdx].att[0] + (pointLights[currLightIdx].att[1] * d) + (pointLights[currLightIdx].att[2] * (d*d))); // Light's falloff factor
        }
    }

    outTarget = float4(localLight, .0f);
}
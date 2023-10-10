#include "forwardRendering.hlsl"

cbuffer constBufferLight : register(b2) {
    uint currLightIdx;
    float3 pad_cbl;
}

void ps_lightingLocalPass (
    in float4 vOutPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTarget: SV_TARGET
) {
    // calculate clip space XY Coord
    float x = 2.0f * vOutPosition.x / viewportDimension.x - 1;
    float y = 1 - (2.0f * vOutPosition.y / viewportDimension.y);

    // getting depth from depth buffer
    float4 depth = ObjDepthMap.Load(int3(vOutPosition.xy, 0));

    if (depth.x == 1.f) { // background
        outTarget = float4(.0f, .0f, .0f, .0f);
        return;
    }

    float4 clipPos = float4(x, y, depth.x, 1.0f);
    
    float4 worldPosH = mul(invViewProjection, clipPos); 

    float3 worldPos = worldPosH.xyz / worldPosH.w;

    float4 sampledAlbedo = ObjTexture.Load(int3(vOutPosition.xy, 0));
    float3 sampledNormal = ObjNormMap.Load(int3(vOutPosition.xy, 0)).xyz;

    // vector between light pos and pixel pos
    float3 pixelToLightV = pointLights[currLightIdx].pos - worldPos.xyz;
    float d = length(pixelToLightV);   

    float3 localLight = float3(.0f, .0f, .0f);

    pixelToLightV = pixelToLightV / d; // convert pixelToLightV to an unit vector
    float cosAngle = dot(pixelToLightV, sampledNormal.xyz); // find the cos(angle) between light and normal

    if (cosAngle > 0.0f) {
        localLight = cosAngle * sampledAlbedo.xyz * pointLights[currLightIdx].diffuse.xyz; // add light to finalColor of pixel
        localLight = localLight / (pointLights[currLightIdx].att.x + (pointLights[currLightIdx].att.y * d) + (pointLights[currLightIdx].att.z * (d*d))); // Light's falloff factor
    }

    outTarget = float4(localLight, 1.f);
}
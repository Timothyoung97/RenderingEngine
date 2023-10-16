#include "forwardRendering.hlsl"

Texture2D ssaoBlurredTexture : register(t7);

void ps_lightingEnvPass (
    in float4 outPosition: SV_POSITION,
    in float2 outTexCoord: TEXCOORD0,
    out float4 outTarget: SV_TARGET
) {

    // calculate clip space XY Coord
    float x = 2.0f * outPosition.x / viewportDimension.x - 1;
    float y = 1 - (2.0f * outPosition.y / viewportDimension.y);

    // getting depth from depth buffer
    float4 depth = ObjDepthMap.Load(int3(outPosition.xy, 0));

    if (depth.x == 1.f) { // background
        outTarget = float4(.0f, .0f, .0f, .0f);
        return;
    }

    float4 clipPos = float4(x, y, depth.x, 1.0f);
    
    float4 worldPosH = mul(invViewProjection, clipPos); 

    float3 worldPos = worldPosH.xyz / worldPosH.w;

    // Sampling gbuffer textures
    float4 sampleAlbedo = ObjTexture.Load(int3(outPosition.xy, 0));
    float3 sampleNormal = ObjNormMap.Load(int3(outPosition.xy, 0)).xyz;
    float4 sampleSSAO = ssaoBlurredTexture.Load(int3(outPosition.xy, 0));

    float3 pixelColor = sampleAlbedo.xyz * .3f * sampleSSAO.x; // hardcoded ambient

    // get dist of pixel from camera
    float3 diff = worldPos.xyz - camPos.xyz;
    float dist = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    // calculate shadow
    float shadow = ShadowCalculation(float4(worldPos, 1.0f), dist);

    pixelColor += (1.0f - shadow) * saturate(dot(dirLight.dir, sampleNormal.xyz)) * dirLight.diffuse.xyz * sampleAlbedo.xyz;

    float3 pixelLightColor = float3(.0f, .0f, .0f);

    // debug colors
    if (csmDebugSwitch) {
        if (dist < planeIntervals[0]) {
            pixelLightColor = float3(.0f, 5.f, .0f);
        } else if (dist < planeIntervals[1] ) {
            pixelLightColor = float3(.0f, .0f, 5.f);
        } else if (dist < planeIntervals[2]) {
            pixelLightColor = float3(5.f, .0f, .5f);
        } else if (dist < planeIntervals[3]){
            pixelLightColor = float3(5.f, .0f, .0f);
        } else {
            pixelLightColor = float3(0.f, .0f, .0f);
        }
    }

    outTarget = float4(pixelColor + pixelLightColor, 0); // RGB + Alpha Channel
}


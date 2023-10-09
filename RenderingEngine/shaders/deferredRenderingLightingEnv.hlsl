#include "forwardRendering.hlsl"

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
    float4 sampleNormal = ObjNormMap.Load(int3(outPosition.xy, 0));

    float3 pixelColor = sampleAlbedo.xyz * .1f; // hardcoded ambient

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

    // read in point light one by one
    // local lighting
    for (int i = 0; i < numPtLights; i++) {
        
        // vector between light pos and pixel pos
        float3 pixelToLightV = pointLights[i].pos - worldPos.xyz;
        float d = length(pixelToLightV);   

        float3 localLight = float3(.0f, .0f, .0f);
    
        if (d <= pointLights[i].range) {
            pixelToLightV = pixelToLightV / d; // convert pixelToLightV to an unit vector
            float cosAngle = dot(pixelToLightV, sampleNormal.xyz); // find the cos(angle) between light and normal

            if (cosAngle > 0.0f) {
                localLight = cosAngle * sampleAlbedo.xyz * pointLights[i].diffuse.xyz; // add light to finalColor of pixel
                localLight = localLight / (pointLights[i].att[0] + (pointLights[i].att[1] * d) + (pointLights[i].att[2] * (d*d))); // Light's falloff factor
            }
        }

        pixelLightColor += localLight;
    }

    outTarget = float4(pixelColor + pixelLightColor, sampleAlbedo.a); // RGB + Alpha Channel
}


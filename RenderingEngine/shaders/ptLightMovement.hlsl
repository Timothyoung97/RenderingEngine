#include "forwardRendering.hlsl"

RWStructuredBuffer<PointLight> ptLights : register(u0);

[numthreads(4, 1, 1)]
void cs_updateLightPosition(uint3 dispatchThreadID : SV_DispatchThreadID) {
    if (dispatchThreadID.x > numPtLights) {
        return;
    }

    PointLight currPtLight = ptLights[dispatchThreadID.x];

    float3 worldOriginToPos = currPtLight.pos;
    float worldOriginToPosLength = length(worldOriginToPos);
    
    float currYaw = ptLights[dispatchThreadID.x].yawPitch.x;
    float currPitch = ptLights[dispatchThreadID.x].yawPitch.y;

    currYaw += dispatchThreadID.x % 2;
    if (currYaw >= 360.f) currYaw = .0f;
    
    currPitch += dispatchThreadID.x % 2;
    if (currPitch >= 360.f) currPitch = .0f;

    float3 newPos;
    newPos.x = worldOriginToPosLength * cos(radians(currYaw)) * cos(radians(currPitch));
    newPos.y = worldOriginToPosLength * sin(radians(currPitch));
    newPos.z = worldOriginToPosLength * sin(radians(currYaw)) * cos(radians(currPitch));;

    ptLights[dispatchThreadID.x].yawPitch = float2(currYaw, currPitch);
    ptLights[dispatchThreadID.x].pos = newPos;
}
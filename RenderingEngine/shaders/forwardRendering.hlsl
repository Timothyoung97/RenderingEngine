#include "helper.hlsl"

struct Light {
    float3 dir;
    float pad;
    float4 ambient;
    float4 diffuse;
};

struct PointLight {
    float3 pos;
    float range;
    float3 att;
    float pad;
    float4 diffuse;
    float2 yawPitch;
    float2 pad2;
};

// Global 
cbuffer constBuffer : register(b0) {
    float2 viewportDimension; // (width, height)
    float2 pad;
    float4 camPos;
    matrix viewProjection;
    matrix invViewProjection;
    matrix lightviewProjection[4];
    float4 planeIntervals;
    Light dirLight;
    uint numPtLights;
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
StructuredBuffer<PointLight> pointLights : register(t2);
Texture2D ObjShadowMap : register(t3);
Texture2D ObjDepthMap : register(t4);

SamplerState ObjSamplerStateLinear : register(s0);
SamplerComparisonState ObjSamplerStateMipPtWhiteBorder : register(s1);

// Vertex Shader
void vs_main (
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
    outPosition = mul(viewProjection, worldPos);

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

static float2 shadowTexCoordCenter[4] = {
    float2(.25f, .25f),
    float2(.75f, .25f),
    float2(.25f, .75f),
    float2(.75f, .75f)
};

static float4 borderClamp[4] = {
    float4(.0f, .49f, .0f, .49f),
    float4(.5f, 1.f, .0f, .49f),
    float4(.0f, .49f, .5f, 1.f),
    float4(.5f, 1.f, .5f, 1.f)
};

static float ShadowCalculation(float4 outWorldPosition, float distFromCamera) {

    float4 pixelPosLightSpace;
    float2 shadowTexCoords;

    [unroll]
    for (int i = 0; i < 4; i++) {
        pixelPosLightSpace = mul(lightviewProjection[i], outWorldPosition);
        shadowTexCoords.x = clamp(shadowTexCoordCenter[i].x + (pixelPosLightSpace.x / pixelPosLightSpace.w * .25f), borderClamp[i].x, borderClamp[i].y);
        shadowTexCoords.y = clamp(shadowTexCoordCenter[i].y - (pixelPosLightSpace.y / pixelPosLightSpace.w * .25f), borderClamp[i].z, borderClamp[i].w);
        if (distFromCamera < planeIntervals[i]) break;
    }

    float pixelDepth = pixelPosLightSpace.z / pixelPosLightSpace.w;

    // convert to 2K
    float2 texelSize = float2(.5f / shadowMapDimension.x, .5f / shadowMapDimension.y);

    float shadow = .0f;
    
    [unroll]
    for (int x = -1; x <= 1; ++x) {
        [unroll]
        for (int y = -1; y <= 1; ++y) {
            float currShadow = ObjShadowMap.SampleCmp(ObjSamplerStateMipPtWhiteBorder, shadowTexCoords.xy + float2(x, y) * texelSize, pixelDepth);
            shadow += currShadow;
        }
    }

    return shadow / 9.f;
};

float4 sampleTexture(float2 textureCoord) {
    float4 sampleTexture = color;
    if (isWithTexture) {
        sampleTexture = ObjTexture.Sample(ObjSamplerStateLinear, textureCoord);
    }
    return sampleTexture;
}

float4 sampleNormal(float2 textureCoord, float4 normal, float4 tangent) {
    normal = normalize(normal);
    if (hasNormMap) {
        float3 normalMap = decodeNormal(ObjNormMap.Sample(ObjSamplerStateLinear, textureCoord).xyz); // change from [0, 1] to [-1, 1]

        // Bitangent TODO: Should be cross(N, T)
        float3 biTangent = normalize(-1.0f * cross(normal.xyz, tangent.xyz)); // create biTangent

        // TBN Matrix
        float4x4 texSpace = {
            tangent,
            float4(biTangent, 0),
            normal,
            float4(.0f, .0f, .0f, 1.0f)
        };

        normal = normalize(mul(float4(normalMap, .0f), texSpace)); // convert normal from normal map to texture space
    }

    return normal;
}

// Pixel Shader
void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTarget: SV_TARGET
) 
{   
    // uv texture
    float4 sampledTexture = sampleTexture(vOutTexCoord);

    // normal
    float4 sampledNormal = sampleNormal(vOutTexCoord, vOutNormal, vOutTangent);

    // init pixel color with directional light
    float3 fColor = sampledTexture.xyz * .1f; // with ambient lighting of directional light (hard coded)

    // get dist of pixel from camera
    float3 diff = outWorldPosition.xyz - camPos.xyz;
    float dist = sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    // calculate shadow
    float shadow = ShadowCalculation(outWorldPosition, dist);

    fColor += (1.0 - shadow) * saturate(dot(dirLight.dir, sampledNormal.xyz)) * dirLight.diffuse.xyz * sampledTexture.xyz;

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

    outTarget = float4(fColor + pixelLightColor, sampledTexture.a); // RGB + Alpha Channel
};
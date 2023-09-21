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
    matrix camViewMatrix;
    matrix viewProjection;
    matrix lightviewProjection[4];
    Light dirLight;
    int numPtLights;
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

float ShadowCalculation(float4 pixelPosLightSpace) {

    float2 shadowTexCoords;
    shadowTexCoords.x = .5f + (pixelPosLightSpace.x / pixelPosLightSpace.w * .5f);
    shadowTexCoords.y = .5f - (pixelPosLightSpace.y / pixelPosLightSpace.w * .5f);

    float pixelDepth = pixelPosLightSpace.z / pixelPosLightSpace.w;

    float shadowMapW, shadowMapH;
    ObjShadowMap.GetDimensions(shadowMapW, shadowMapH);

    float2 texelSize = float2(1.f / shadowMapW, 1.f / shadowMapH);

    float shadow = .0f;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float currShadow = ObjShadowMap.SampleCmp(ObjSamplerStateMipPtWhiteBorder, shadowTexCoords.xy + float2(x, y) * texelSize, pixelDepth);
            shadow += currShadow;
        }
    }

    return shadow / 9.f;
};

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
    // TODO: Calculate vOutNormal from SV_POSITION

    // normal
    vOutNormal = normalize(vOutNormal);

    // uv texture
    float4 sampleTexture;

    if (isWithTexture) {
        sampleTexture = ObjTexture.Sample(ObjSamplerStateLinear, vOutTexCoord);
    } else {
        sampleTexture = color;
    }

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

        vOutNormal = normalize(mul(normalMap, texSpace)); // convert normal from normal map to texture space
    }

    // init pixel color with directional light
    float3 fColor = sampleTexture.xyz * .1f; // with ambient lighting of directional light (hard coded)

    float4 pixelPosLightSpace = mul(lightviewProjection[0], outWorldPosition);

    float shadow = ShadowCalculation(pixelPosLightSpace);

    fColor += (1.0 -shadow) * saturate(dot(dirLight.dir, vOutNormal.xyz)) * dirLight.diffuse.xyz * sampleTexture.xyz;

    float3 pixelLightColor = float3(.0f, .0f, .0f);

    // read in point light one by one
    // local lighting
    for (int i = 0; i < numPtLights; i++) {
        
        // vector between light pos and pixel pos
        float3 pixelToLightV = pointLights[i].pos - outWorldPosition.xyz;
        float d = length(pixelToLightV);   

        float3 localLight = float3(.0f, .0f, .0f);
    
        if (d <= pointLights[i].range) {
            pixelToLightV = pixelToLightV / d; // convert pixelToLightV to an unit vector
            float cosAngle = dot(pixelToLightV, vOutNormal.xyz); // find the cos(angle) between light and normal

            if (cosAngle > 0.0f) {
                localLight = cosAngle * sampleTexture.xyz * pointLights[i].diffuse.xyz; // add light to finalColor of pixel
                localLight = localLight / (pointLights[i].att[0] + (pointLights[i].att[1] * d) + (pointLights[i].att[2] * (d*d))); // Light's falloff factor
            }
        }

        pixelLightColor += localLight;
    }

    outTarget = float4(fColor + pixelLightColor, sampleTexture.a); // RGB + Alpha Channel
};
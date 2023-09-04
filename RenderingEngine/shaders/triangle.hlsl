struct Light {
    float3 dir;
    float3 pos;
    float range;
    float3 att;
    float4 ambient;
    float4 diffuse;
};

// Global 
cbuffer constBuffer : register(b0) {
    matrix viewProjection;
    Light light;
};

// Per Object
cbuffer constBuffer2 : register(b1) {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
};

Texture2D ObjTexture;
SamplerState ObjSamplerState;

// Vertex Shader
void vs_main (
    in float3 inPosition : POSITION,
    in float3 inNormal : NORMAL,
    in float2 inTexCoord : TEXCOORD,
    out float4 outPosition : SV_POSITION,
    out float4 outNormal : NORMAL,
    out float2 outTexCoord : TEXCOORD
) 
{   
    // Position
    float4 tempInPos = float4(inPosition, 1);
    float4 localPos = mul(transformation, tempInPos);
    outPosition = mul(viewProjection, localPos);

    // Normal
    float4 tempInNormal = float4(inNormal, 0);
    outNormal = mul(normalMatrix, tempInNormal);

    // Texture
    outTexCoord = inTexCoord;
};

// Pixel Shader
void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 vOutNormal : NORMAL,
    in float2 vOutTexCoord : TEXCOORD,
    out float4 outTarget: SV_TARGET
) 
{   
    // normal
    vOutNormal = normalize(vOutNormal);

    // uv texture
    float4 sampleTexture;
    if (isWithTexture) {
        sampleTexture = ObjTexture.Sample(ObjSamplerState, vOutTexCoord);
    } else {
        sampleTexture = color;
    }

    // init pixel color
    float3 fColor = float3(0.0f, 0.0f, 0.0f);

    // vector between light pos and pixel pos
    float3 PixelToLightV = light.pos - vOutPosition.xyz; 
    float d = length(PixelToLightV);

    // ambient
    float3 fAmbient = sampleTexture.xyz * 0.1f;

    // out of range pixel
    if (d > light.range) {
        outTarget = float4(fAmbient, sampleTexture.a);
        return;
    }

    PixelToLightV /= d; // convert PixelToLightV to an unit vector
    float cosAngle = dot(PixelToLightV, vOutNormal.xyz); // find the cos(angle) between light and normal

    if (cosAngle > 0.0f) {
        fColor += cosAngle * sampleTexture.xyz * light.diffuse.xyz; // add light to finalColor of pixel
        fColor /= light.att[0] + (light.att[1] * d) + (light.att[2] * (d*d)); // Light's falloff factor
    }

    fColor = saturate(fColor + sampleTexture.xyz * 0.1); // add ambient light (using hardcoded ambient light)

    outTarget = float4(fColor, sampleTexture.a); // RGB + Alpha Channel
};
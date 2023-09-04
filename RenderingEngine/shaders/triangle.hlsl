struct Light {
    float3 dir;
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

    // Final Color
    float3 fColor;
    // fColor = sampleTexture.xyz * light.ambient.xyz;
    fColor = saturate(dot(light.dir, vOutNormal.xyz)) * light.diffuse.xyz * sampleTexture.xyz;

    outTarget = float4(fColor, sampleTexture.a); // RGB + Alpha Channel
};
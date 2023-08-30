cbuffer constBuffer : register(b0) {
    matrix viewProjection;
}

cbuffer constBuffer2 : register(b1) {
    matrix transformation; 
}

Texture2D ObjTexture;
SamplerState ObjSamplerState;

// Vertex Shader
void vs_main (
    in float3 inPosition : POSITION,
    in float2 inTexCoord : TEXCOORD,
    out float4 outPosition : SV_POSITION,
    out float2 outTexCoord : TEXCOORD
) 
{   
    float4 tempInPos = float4(inPosition, 1);
    float4 localPos = mul(transformation, tempInPos);
    outPosition = mul(viewProjection, localPos);

    outTexCoord = inTexCoord;
}

// Pixel Shader
void ps_main (
    in float4 outPosition : SV_POSITION,
    in float2 vOutTexCoord : TEXCOORD,
    out float4 outTarget: SV_TARGET
) 
{
    outTarget = ObjTexture.Sample(ObjSamplerState, vOutTexCoord);
}
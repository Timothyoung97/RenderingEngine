cbuffer constBuffer : register(b0) {
    matrix transformation; 
    matrix viewProjection;
}

// Vertex Shader
void vs_main (
    in float3 inPosition : POSITION,
    in float4 vInColor : COLOR,
    out float4 outPosition : SV_POSITION,
    out float4 vOutColor : TEXCOORD0
) 
{   
    float4 tempInPos = float4(inPosition, 1);
    float4 localPos = mul(transformation, tempInPos);
    outPosition = mul(viewProjection, localPos);

    vOutColor = vInColor;
}

// Pixel Shader
void ps_main (
    in float4 outPosition : SV_POSITION,
    in float4 vOutColor : TEXCOORD0,
    out float4 pOutColor: SV_TARGET
) 
{
    pOutColor = vOutColor;
}
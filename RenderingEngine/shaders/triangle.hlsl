cbuffer constBuffer : register(b0) {
    matrix transformation; 
    matrix viewProjection;
    float4 color;
}

void vs_main (
    in float4 inPosition : POSITION,
    out float4 outPosition : SV_POSITION
) 
{
    float4 localPos = mul(transformation, inPosition);
    outPosition = mul(viewProjection, localPos);
}

// Pixel Shader
void ps_main (
    out float4 outColor: SV_TARGET
) 
{
    outColor = color;
}
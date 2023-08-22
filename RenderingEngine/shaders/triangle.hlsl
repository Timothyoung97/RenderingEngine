cbuffer constBuffer : register(b0) {
    matrix transformation; 
    matrix viewProjection;
    float4 color;
}

void vs_main (
    in uint vertexID : SV_VertexID,
    out float4 outPosition : SV_POSITION
) 
{
    const float4 pos[3] = {
        float4(0, 0.5, 0, 1),
        float4(0.5, -0.5, 0, 1),
        float4(-0.5, -0.5, 0, 1)
    };

    float4 localPos = mul(transformation, pos[vertexID]);
    outPosition = mul(viewProjection, localPos);
}

// Pixel Shader
void ps_main (
    out float4 outColor: SV_TARGET
) 
{
    outColor = color;
}
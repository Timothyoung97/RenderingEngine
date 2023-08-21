cbuffer constBuffer : register(b0) {matrix transformation;}

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

    outPosition = mul(transformation, pos[vertexID]);
}

// Pixel Shader
void ps_main (
    out float4 outColor: SV_TARGET
) 
{
    outColor = float4(1.0, 0.0, 0.0, 1.0); // Red color (R, G, B, A)
}
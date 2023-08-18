cbuffer constBuffer : register(b0) {float2 vertexOffset;}

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

    outPosition = pos[vertexID] + float4(vertexOffset, 0, 0);
}

// Pixel Shader
void ps_main (
    out float4 outColor: SV_TARGET
) 
{
    outColor = float4(1.0, 0.0, 0.0, 1.0); // Red color (R, G, B, A)
}
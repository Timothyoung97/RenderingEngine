void vs_main (
    in uint VertexID : SV_VertexID,
    out float4 OutPosition : SV_POSITION
) 
{
    const float4 pos[3] = {
        float4(0, .75, 0, 1),
        float4(.5, 0, 0, 1),
        float4(0, 0, .5, 1)
    };

    OutPosition = pos[VertexID];
}

// Pixel Shader
void ps_main (
    out float4 OutColor: SV_TARGET
) 
{
    OutColor = float4(1.0, 0.0, 0.0, 1.0); // Red color (R, G, B, A)
}
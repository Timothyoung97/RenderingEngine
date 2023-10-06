static const float4 vertices[6] = {
    float4(-1.f,  1.f, 0.f, 1.f), 
    float4(-1.f, -1.f, 0.f, 1.f), 
    float4( 1.f,  1.f, 0.f, 1.f), 
    float4(-1.f, -1.f, 0.f, 1.f), 
    float4( 1.f, -1.f, 0.f, 1.f),
    float4( 1.f,  1.f, 0.f, 1.f) 
};

static const float2 uv[6] = {
    float2(0, 0),
    float2(0, 1),
    float2(1, 0),
    float2(0, 1),
    float2(1, 1),
    float2(1, 0)
};

void vs_fullscreen(
    in uint vertexID: SV_VertexID,
    out float4 outPosition: SV_POSITION,
    out float2 outTexCoord: TEXCOORD0
) {
    outPosition = vertices[vertexID];
    outTexCoord = uv[vertexID];
}
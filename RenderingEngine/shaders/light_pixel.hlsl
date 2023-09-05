void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 vOutLocalPosition : POSITION,
    in float4 vOutNormal : NORMAL,
    in float2 vOutTexCoord : TEXCOORD,
    out float4 outTarget: SV_TARGET
)
{
    outTarget = float4(1.0f, 1.0f, 1.0f, 1.0f);
};
void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 vOutLocalPosition : POSITION,
    in float4 vOutNormal : NORMAL,
    in float4 vOutTangent : TANGENT,
    in float4 vOutBitangent : TEXCOORD0,
    in float2 vOutTexCoord : TEXCOORD1,
    out float4 outTarget: SV_TARGET
)
{
    outTarget = float4(1.0f, 1.0f, 1.0f, 1.0f);
};
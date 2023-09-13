// Per Object
cbuffer constBuffer2 : register(b1) {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
    uint hasNormMap;
};

void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 vOutLocalPosition : TEXCOORD0,
    in float4 vOutNormal : TEXCOORD1,
    in float4 vOutTangent : TEXCOORD2,
    in float2 vOutTexCoord : TEXCOORD3,
    out float4 outTarget: SV_TARGET
)
{
    outTarget = color;
};
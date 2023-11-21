void ps_instanced_wireframe (
    in float4 outPosition : SV_POSITION,
    in float4 outWorldPosition : TEXCOORD0,
    in float4 outNormal : TEXCOORD1,
    in float4 outTangent : TEXCOORD2,
    in float2 outTexCoord : TEXCOORD3,
    in float4 outColor : TEXCOORD4,
    in float4 outTextuerInfo : TEXCOORD5,
    out float4 outTarget: SV_TARGET
) {
    outTarget = outColor;
}

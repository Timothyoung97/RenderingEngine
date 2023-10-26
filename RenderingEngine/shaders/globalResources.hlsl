struct Light {
    float3 dir;
    float pad;
    float4 ambient;
    float4 diffuse;
};

struct PointLight {
    float3 pos;
    float range;
    float3 att;
    float pad;
    float4 diffuse;
    float2 yawPitch;
    float2 pad2;
};
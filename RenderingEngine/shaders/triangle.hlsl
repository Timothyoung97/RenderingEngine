struct Light {
    float3 dir;
    float4 ambient;
    float4 diffuse;
};

struct PointLight {
    float3 dir;
    float3 pos;
    float range;
    float3 att;
    float4 ambient;
    float4 diffuse;
};

// Global 
cbuffer constBuffer : register(b0) {
    matrix viewProjection;
    Light dirLight;
    PointLight pointLight[4];
};

// Per Object
cbuffer constBuffer2 : register(b1) {
    matrix transformation;
    matrix normalMatrix;
    float4 color;
    uint isWithTexture;
    uint hasNormMap;
};

Texture2D ObjTexture;
Texture2D ObjNormMap;
SamplerState ObjSamplerState;

// Vertex Shader
void vs_main (
    in float3 inPosition : POSITION,
    in float3 inNormal : NORMAL,
    in float3 inTangent : TANGENT,
    in float2 inTexCoord : TEXCOORD,
    out float4 outPosition : SV_POSITION,
    out float4 outLocalPosition : POSITION,
    out float4 outNormal : NORMAL,
    out float4 outTangent : TANGENT,
    out float2 outTexCoord : TEXCOORD
) 
{   
    // Position
    float4 tempInPos = float4(inPosition, 1);
    float4 localPos = mul(transformation, tempInPos);
    outPosition = mul(viewProjection, localPos);

    // local position
    outLocalPosition = localPos;

    // Normal
    float4 tempInNormal = float4(inNormal, 0);
    outNormal = mul(normalMatrix, tempInNormal);

    // Tangernt
    float4 tempInTangent = float4(inTangent, 0);
    outTangent = mul(normalMatrix, tempInTangent); // using normal matrix to move tangent

    // Texture
    outTexCoord = inTexCoord;
};

// Pixel Shader
void ps_main (
    in float4 vOutPosition : SV_POSITION,
    in float4 vOutLocalPosition : POSITION,
    in float4 vOutNormal : NORMAL,
    in float4 vOutTangent : TANGENT,
    in float2 vOutTexCoord : TEXCOORD,
    out float4 outTarget: SV_TARGET
) 
{   
    // TODO: Calculate vOutNormal from SV_POSITION

    // normal
    vOutNormal = normalize(vOutNormal);

    // uv texture
    float4 sampleTexture;

    if (isWithTexture) {
        sampleTexture = ObjTexture.Sample(ObjSamplerState, vOutTexCoord);
    } else {
        sampleTexture = color;
    }

    if (hasNormMap) {
        float4 normalMap = ObjNormMap.Sample(ObjSamplerState, vOutTexCoord);

        normalMap = (2.0f * normalMap) - 1.0f; // change from [0, 1] to [-1, 1]

        vOutTangent = normalize(vOutTangent - dot(vOutTangent, vOutNormal) * vOutNormal); // ensure tangent is orthogonal to normal

        float3 biTangent = cross(vOutNormal.xyz, vOutTangent.xyz); // create biTangent

        // create texture space
        float4x4 texSpace = float4x4(
            vOutTangent, 
            float4(biTangent, 0), 
            vOutNormal, 
            float4(.0f, .0f, .0f, 1.0f)
        );
        
        vOutNormal = normalize(mul(normalMap, texSpace)); // convert normal from normal map to texture space
    }

    // init pixel color with directional light
    float3 fColor = saturate(dot(dirLight.dir, vOutNormal.xyz)) * dirLight.diffuse.xyz * sampleTexture.xyz;
    fColor += sampleTexture.xyz * .1f; // with ambient lighting of directional light (hard coded)

    float3 pixelLightColor = float3(.0f, .0f, .0f);

    // local lighting
    for (int i = 0; i < 4; i++) {

        // vector between light pos and pixel pos
        float3 pixelToLightV = pointLight[i].pos - vOutLocalPosition.xyz;
        float d = length(pixelToLightV);   

        float3 localLight = float3(.0f, .0f, .0f);
    
        if (d <= pointLight[i].range) {
            pixelToLightV = pixelToLightV / d; // convert pixelToLightV to an unit vector
            float cosAngle = dot(pixelToLightV, vOutNormal.xyz); // find the cos(angle) between light and normal

            if (cosAngle > 0.0f) {
                localLight = cosAngle * sampleTexture.xyz * pointLight[i].diffuse.xyz; // add light to finalColor of pixel
                localLight = localLight / (pointLight[i].att[0] + (pointLight[i].att[1] * d) + (pointLight[i].att[2] * (d*d))); // Light's falloff factor
            }
        }

        pixelLightColor += localLight;
    }

    outTarget = float4(fColor + pixelLightColor, sampleTexture.a); // RGB + Alpha Channel
};
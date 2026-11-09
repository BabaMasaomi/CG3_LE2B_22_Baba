#include "Object3d.hlsli"

struct Material
{
    float32_t4 color;
    int32_t enableLighting;
    float3 padding;
    float32_t4x4 uvTransform;
};

struct DirectionalLight
{
    float32_t4 color;
    float32_t3 direction;
    float intensity;
};

struct ObjectLighting
{
    int32_t lightingType;
    float3 padding;
};

ConstantBuffer<Material> gMaterial : register(b0);
ConstantBuffer<DirectionalLight> gDirectionalLight : register(b1);
ConstantBuffer<ObjectLighting> gObjectLighting : register(b2);

Texture2D<float32_t4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float32_t4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;

    // UVTransformを適用
    float4 transformedUV = mul(float32_t4(input.texcoord, 0.0f, 1.0f), gMaterial.uvTransform);
    // テクスチャ色を取得
    float32_t4 textureColor = gTexture.Sample(gSampler, transformedUV.xy);
    // マテリアル色とテクスチャ色を合成
    float32_t4 baseColor = gMaterial.color * textureColor;

    // ライティング無効、またはLightingTypeがNone
    if (gMaterial.enableLighting == 0 || gObjectLighting.lightingType == 0)
    {
        output.color = baseColor;
        return output;
    }

    // 法線とライト方向を正規化
    float3 normal = normalize(input.normal);
    float3 lightDirection = normalize(-gDirectionalLight.direction);
    float NdotL = dot(normal, lightDirection);
    float diffuse = 1.0f;

    // Lambert
    if (gObjectLighting.lightingType == 1)
    {
        diffuse = saturate(NdotL);
    }
    // Half Lambert
    else if (gObjectLighting.lightingType == 2)
    {
        diffuse = pow(saturate(NdotL * 0.5f + 0.5f), 2.0f);
    }

    // RGBだけにライティングを適用
    output.color.rgb = baseColor.rgb * gDirectionalLight.color.rgb * diffuse * gDirectionalLight.intensity;
    // αにはライティングを適用しない
    output.color.a = baseColor.a;

    return output;
}
struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer LineMaterial : register(b0)
{
    float4 materialColor;
};

PixelShaderOutput main()
{
    PixelShaderOutput output;

    output.color = materialColor;

    return output;
}
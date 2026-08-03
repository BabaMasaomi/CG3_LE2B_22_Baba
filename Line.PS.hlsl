struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer LineMaterial : register(b0)
{
    float4 materialColor;
};

//ConstantBuffer<LineMaterial>
  //  gMaterial : register(b0);

PixelShaderOutput main()
{
    PixelShaderOutput output;

    output.color = materialColor;

    return output;
}
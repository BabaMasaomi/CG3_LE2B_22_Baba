struct VertexShaderInput
{
    float4 position : POSITION0;
};

struct VertexShaderOutput
{
    float4 position : SV_POSITION;
};

struct TransformationMatrix
{
    float4x4 WVP;
    float4x4 World;
};

cbuffer TransformationMatrix : register(b1)
{
    float4x4 WVP;
    float4x4 World;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;

    output.position = mul(
        input.position,
        WVP
    );

    return output;
}
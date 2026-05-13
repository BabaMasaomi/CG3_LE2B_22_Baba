

struct PixelShaderOutput{
    float32_t4 color : SV_TARGET0;
};

float32_t4 main(PixelShaderOutput input){
    PixelShaderOutput output;
    output.color = float32_t4(1.0f, 1.0f, 1.0f, 1.0f);
    return output;
}
struct PSInput
{
    float4 position     : SV_POSITION;
    float2 texCoord     : TEXCOORD;
    float2 tilingFactor : TILINGFACTOR;
    float4 color        : COLOR;
    uint texIndex       : TEXINDEX;
};

Texture2D textures[]    : register(t0);
SamplerState samplerState : register(s0);

struct PSOutput
{
    float4 color : SV_TARGET0;
};

PSOutput main(PSInput input)
{
    float4 texColor = textures[input.texIndex].Sample(samplerState, input.texCoord * input.tilingFactor);
    float4 finalColor = input.color * texColor;
    
    // Discard pixel if alpha is zero
    clip(finalColor.a == 0.0f ? -1.0f : 1.0f);
    
    PSOutput result;
    result.color = finalColor;

    return result;
}
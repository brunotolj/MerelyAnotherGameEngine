
struct VSInput
{
    float Index;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 FragUV;
};

struct PushConstants
{
    float4 ScreenCoords;
    float4 TextureCoords;
};

struct UniformBuffer
{
    float4 ScreenTransform;
};

[[vk::push_constant]] PushConstants push;

[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(1, 0)]] Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;

    float2 localCoords = float2(fmod(input.Index, 2.0), floor(input.Index / 2.0));
    float2 screenCoords = push.ScreenCoords.xy + localCoords * push.ScreenCoords.zw;

    output.Position.xy = ubo.ScreenTransform.xy + screenCoords * ubo.ScreenTransform.zw;
    output.Position.z = 0.0;
    output.Position.w = 1.0;

    output.FragUV = push.TextureCoords.xy + localCoords * push.TextureCoords.zw;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input) : SV_Target
{
    return float4(texture.Sample(input.FragUV).rgb, 1.0);
}


struct VSInput
{
    float3 Position;
    float3 Normal;
    float2 TextureCoords;
};

struct VSOutput
{
    float4 Position : SV_Position;
    float2 LightIntensity;
    float2 TextureCoords;
};

struct PushConstants
{
    float4x4 Transform;
};

struct UniformBuffer
{
    float4x4 CameraTransform;
    float4 LightDirectionAndAmbient;
};

[[vk::push_constant]] PushConstants push;

[[vk::binding(0, 0)]] ConstantBuffer<UniformBuffer> ubo;

[[vk::binding(1, 0)]] Sampler2D texture;

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;

    output.Position = mul(ubo.CameraTransform, mul(push.Transform, float4(input.Position, 1.0)));

    float4 transformedNormal = mul(push.Transform, float4(input.Normal, 0.0));
    transformedNormal.w = 0.0;
    transformedNormal = normalize(transformedNormal);
    output.LightIntensity.x = ubo.LightDirectionAndAmbient.w;
    output.LightIntensity.y = -dot(transformedNormal, ubo.LightDirectionAndAmbient);

    output.TextureCoords = input.TextureCoords;

    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput input)
{
    float light = input.LightIntensity.x + max(0.0, input.LightIntensity.y);
    float4 textureColor = float4(texture.Sample(input.TextureCoords).rgb, 1.0);
    return float4((light * textureColor).rgb, 1.0);
}

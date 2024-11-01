#version 460

layout (location = 0) in vec2 lightIntensity;
layout (location = 1) in vec2 fragUV;

layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D image;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec4 Color;
} push;

void main()
{
	float light = lightIntensity.x + max(0.0, lightIntensity.y);
	vec4 imageColor = vec4(texture(image, fragUV).rgb, 1.0);
	color = light * push.Color * imageColor;
}

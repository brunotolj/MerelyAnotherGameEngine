#version 460

layout (location = 0) in vec2 lightIntensity;

layout (location = 0) out vec4 color;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec4 Color;
} push;

void main()
{
	float light = lightIntensity.x + max(0.0, lightIntensity.y);
	color = light * push.Color;
}

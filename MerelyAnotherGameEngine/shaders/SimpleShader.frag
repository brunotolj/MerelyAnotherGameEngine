#version 460

layout (location = 0) out vec4 color;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec3 Color;
} push;

void main()
{
	color = vec4(push.Color, 1.0);
}
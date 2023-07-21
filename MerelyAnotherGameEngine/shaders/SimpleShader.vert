#version 460

layout (location = 0) in vec3 position;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec3 Color;
} push;

void main()
{
	gl_Position = push.Transform * vec4(position, 1.0);
}
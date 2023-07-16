#version 460

layout (location = 0) in vec2 position;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec3 Color;
} push;

void main()
{
	gl_Position = push.Transform * vec4(position, 0.0, 1.0);
}
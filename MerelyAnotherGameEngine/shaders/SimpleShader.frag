#version 460

layout (location = 0) in vec4 smoothNormal;
layout (location = 1) in vec4 objectColor;

layout (location = 0) out vec4 color;

layout(push_constant) uniform Push
{
	mat4 Transform;
	mat4 NormalMatrix;
} push;

const vec4 LIGHT_DIRECTION = normalize(vec4(3.0, 2.0, -2.0, 0.0));
const float AMBIENT = 0.05;

void main()
{
	const float dotResult = -dot(LIGHT_DIRECTION, smoothNormal);
	float factor = AMBIENT + max(dotResult, 0.0);
	color = factor * objectColor;
}
#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec4 transformedNormal;
layout (location = 1) out vec4 objectColor;

layout(push_constant) uniform Push
{
	mat4 Transform;
	mat4 NormalMatrix;
} push;

void main()
{
	transformedNormal = normalize(push.NormalMatrix * vec4(normal, 0.0));
	objectColor = push.NormalMatrix[3];
	gl_Position = push.Transform * vec4(position, 1.0);
}
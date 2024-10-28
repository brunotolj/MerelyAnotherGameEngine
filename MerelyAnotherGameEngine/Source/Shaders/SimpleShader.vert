#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec2 lightIntensity;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	mat4 CameraTransform;
	vec4 LightDirectionAndAmbient;
} ubo;

layout(push_constant) uniform Push
{
	mat4 Transform;
	vec4 Color;
} push;

void main()
{
	gl_Position = ubo.CameraTransform * push.Transform * vec4(position, 1.0);
	
	vec4 transformedNormal = push.Transform * vec4(normal, 0.0);
	transformedNormal.w = 0.0;
	transformedNormal = normalize(transformedNormal);
	lightIntensity.x = -dot(transformedNormal, ubo.LightDirectionAndAmbient);
	lightIntensity.y = ubo.LightDirectionAndAmbient.w;
}

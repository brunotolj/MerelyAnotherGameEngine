#version 460

layout (location = 0) in float index;

layout (location = 0) out vec2 fragUV;

layout (set = 0, binding = 0) uniform GlobalUBO
{
	vec4 ScreenTransform;
} ubo;

layout(push_constant) uniform Push
{
	vec4 ScreenCoords;
	vec4 TextureCoords;
} push;

void main()
{
	vec2 localCoords = vec2(mod(index, 2.0), floor(index / 2.0));

	vec2 screenCoords = push.ScreenCoords.xy + localCoords * push.ScreenCoords.zw;
	gl_Position.xy = ubo.ScreenTransform.xy + screenCoords * ubo.ScreenTransform.zw;
	
	fragUV = push.TextureCoords.xy + localCoords * push.TextureCoords.zw;

	gl_Position.z = 0.0;
	gl_Position.w = 1.0;
}

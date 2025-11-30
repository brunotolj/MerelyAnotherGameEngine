#version 460

layout (location = 0) in vec2 fragUV;

layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D image;

layout(push_constant) uniform Push
{
	vec4 ScreenCoords;
	vec4 TextureCoords;
} push;

void main()
{
	color = vec4(texture(image, fragUV).rgb, 1.0);
}

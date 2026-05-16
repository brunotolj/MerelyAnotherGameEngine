#include "Assets/TextureFactory.h"

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

AssetHandle<Texture> Factory<Texture>::FromFile(mage::StringView inPath, Vulkan::Renderer const& inRenderer, AssetManager& inAssetManager)
{
	Texture* result = new Texture();

	i32 bytesPerPixel;

	stbi_uc* imageData = stbi_load(inPath.GetCString(), reinterpret_cast<i32*>(&result->mSize.width), reinterpret_cast<i32*>(&result->mSize.height), &bytesPerPixel, 4);
	u32 imageSize = result->mSize.width * result->mSize.height * 4;

	result->mData.ResizeUninitialized(imageSize);
	memcpy(result->mData.GetData(), imageData, imageSize);

	result->mSize.depth = 1;

	stbi_image_free(imageData);

	result->CreateImage(inRenderer);

	return inAssetManager.Register(result);
}

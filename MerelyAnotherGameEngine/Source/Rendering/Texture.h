#pragma once

#include "Core/NonCopyable.h"
#include "Rendering/Device.h"

#include <string>

class Texture : public NonCopyableClass
{
public:
	Texture(Device& device, const std::string& filePath);
	~Texture();

	VkSampler GetSampler() { return mSampler; }
	VkImageView GetImageView() { return mImageView; }
	VkImageLayout GetImageLayout() { return mImageLayout; }

	VkDescriptorImageInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

private:
	Device& mDevice;
	VkImage mImage;
	VkDeviceMemory mImageMemory;
	VkImageView mImageView;
	VkSampler mSampler;
	VkFormat mImageFormat;
	VkImageLayout mImageLayout;

	void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
};

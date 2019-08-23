#pragma once

#ifndef TEXTUREDATA_DEFINED
#define TEXTUREDATA_DEFINED

#include "VulkanUtilities.h"
#include "Vulkan/vulkan.h"
#include "Optional.h"

struct textureCreationInfo
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkFormat format;
	VkImageAspectFlags aspectFlags;
	const char *path;
	bool hasSampler;
};

class TextureData
{
public:

	TextureData(){}
	TextureData(textureCreationInfo creationInfo);

	void cleanup(VkDevice device);

	void createTextureSampler(VkDevice device);

	void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const char * path);

	void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

	void setView(VkImageView givenView);

	VkImage getImage();

	Optional<VkSampler> getSampler();

	VkImageView getView();

	static void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);


private:

	
	static bool hasStencilComponent(VkFormat format);

	static void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VkImageView view = nullptr;
	Optional<VkSampler> sampler;
};

#endif // !TEXTUREDATA_DEFINED




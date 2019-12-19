#pragma once

#ifndef TEXTUREDATA_DEFINED
#define TEXTUREDATA_DEFINED

#include "VulkanUtilities.h"
#include "Vulkan/vulkan.h"
#include "Optional.h"
#include <string>

struct textureCreationInfo
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkFormat format;
	VkImageAspectFlags aspectFlags;
	bool hasSampler;
};

class TextureData
{
public:

	TextureData(){}
	TextureData(textureCreationInfo creationInfo, std::string path);

	static TextureData create(std::string name);

	static void destroy(TextureData &givenTextureData);

	void cleanup(VkDevice device);

	void createTextureSampler(VkDevice device);

	void createTextureImage(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue, const char * path);

	void createImage(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);

	void setView(VkImageView givenView);

	VkImage getImage();

	uint32_t getMipLevels() { return mipLevels; }

	Optional<VkSampler> getSampler() const;

	VkImageView getView() const;

	static void transitionImageLayout(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

private:

	
	static bool hasStencilComponent(VkFormat format);

	static void copyBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	void generateMipmaps(VkDevice device, VkCommandPool commandPool, VkQueue graphicsQueue, VkImage image, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);


	uint32_t mipLevels = 0;
	VkImage image = nullptr;
	VkDeviceMemory imageMemory = nullptr;
	VkImageView view = nullptr;
	Optional<VkSampler> sampler;
};

#endif // !TEXTUREDATA_DEFINED




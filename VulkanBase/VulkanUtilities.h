#pragma once

#ifndef UTILITIES_DEFINED
#define UTILITIES_DEFINED

#include <set>
#include "Vulkan/vulkan.h"
#include <vector>

struct bufferCreationInfo
{
	VkDevice device;
	VkDeviceSize size;
	VkBufferUsageFlags usage;
	VkMemoryPropertyFlags properties;
	VkPhysicalDevice physicalDevice;
};

//class VkUtils
//{
//
//private:
//	VkUtils(){};
//};

template <class T>
static T min(const T &a, const T &b)
{
	return (a < b) ? a : b;
}

template <class T>
static T max(const T &a, const T &b)
{
	return (a > b) ? a : b;
}

#pragma region DEBUGGING

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

#pragma endregion


VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

void readFile(const std::string& filename, std::vector<char> *buffer);

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool pool);

void endSingleTimeCommands(VkDevice device, VkCommandPool pool, VkQueue graphicsQueue, VkCommandBuffer commandBufferData);

void copyBuffer(VkDevice device, VkCommandPool pool, VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

void createBuffer(
	bufferCreationInfo creationInfo,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory
);

#endif // !UTILITIES_DEFINED
#pragma once
#ifndef UNIFORMDATA_DEFINED
#define UNIFORMDATA_DEFINED

#include "TextureData.h"
#include "VulkanUtilities.h"
#include "Renderer.h"
#include "InputManager.h"
#include "Time.h"
#include <stdexcept>

#pragma warning(disable: 6386)


struct alignas(16) GBufferUBO
{
	glm::mat4 model = glm::mat4(1.0f);								
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);
	glm::vec4 color = glm::vec4();
};

struct uniformDataCreationInfo
{
	uniformDataCreationInfo(SwapChainData &givenSwapChainData) : swapChainData(givenSwapChainData){}
	VkDevice device = {};
	VkPhysicalDevice physicalDevice = {};
	SwapChainData &swapChainData;
	VkDescriptorSetLayout descriptorSetLayout = {};
	TextureData texture = {};
};

class UniformData
{
public:

	UniformData(const uniformDataCreationInfo &creationInfo)
	{
		createData(creationInfo.device, creationInfo.physicalDevice, creationInfo.swapChainData, creationInfo.descriptorSetLayout, creationInfo.texture);
	}
	UniformData() = default;

	void createData(VkDevice device, VkPhysicalDevice physicalDevice, const SwapChainData &swapChainData, VkDescriptorSetLayout descriptorSetLayout, const TextureData &texture)
	{
		createUniformBuffers(device, physicalDevice, swapChainData);
		createDescriptorPool(device, swapChainData);
		createDescriptorSets(device, swapChainData, descriptorSetLayout, texture);
	}

	void createDescriptorPool(VkDevice device, const SwapChainData &swapChainData)
	{
		constexpr uint32_t poolSizesCount = 2;
		VkDescriptorPoolSize *poolSizes = new VkDescriptorPoolSize[poolSizesCount];
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizesCount;
		poolInfo.pPoolSizes = poolSizes;
		poolInfo.maxSets = swapChainData.imagesCount;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
		delete[] poolSizes;
	}

	void createUniformBuffers(VkDevice device, VkPhysicalDevice physicalDevice, const SwapChainData &swapChainData) {
		VkDeviceSize bufferSize = sizeof(GBufferUBO);
		uniformBuffers.resize(swapChainData.imagesCount);
		uniformBuffersMemory.resize(swapChainData.imagesCount);

		for (size_t i = 0; i < swapChainData.imagesCount; i++) {
			bufferCreationInfo creationInfo;
			creationInfo.size = bufferSize;
			creationInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			creationInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
			creationInfo.physicalDevice = physicalDevice;
			creationInfo.device = device;
			createBuffer(creationInfo, uniformBuffers[i], uniformBuffersMemory[i]);
		}
	}

	void createDescriptorSets(VkDevice device, const SwapChainData &swapChainData, VkDescriptorSetLayout descriptorSetLayout, const TextureData &texture)
	{
		std::vector<VkDescriptorSetLayout> layouts;
		for (unsigned int i = 0; i < swapChainData.imagesCount; i++)
		{
			layouts.push_back(descriptorSetLayout);
		}

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainData.imagesCount);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(swapChainData.imagesCount);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		descriptorSets.resize(swapChainData.imagesCount);

		for (size_t i = 0; i < swapChainData.imagesCount; i++)
		{

			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(GBufferUBO);

			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfo.imageView = texture.getView();
			if (!texture.getSampler().isSet()) throw std::exception("Texture sampler not set!");
			imageInfo.sampler = texture.getSampler().getValue();

			constexpr uint32_t descriptorWritesCount = 2;
			VkWriteDescriptorSet descriptorWrites[descriptorWritesCount] = {};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].dstArrayElement = 0;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(device, descriptorWritesCount, descriptorWrites, 0, nullptr);
		}
	}

	void cleanup(VkDevice device)
	{
		for (size_t i = 0; i < uniformBuffers.size(); i++) {
			vkDestroyBuffer(device, uniformBuffers[i], nullptr);
			vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
		}

		uniformBuffers.clear();
		uniformBuffersMemory.clear();

		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}

	VkDescriptorPool getDescriptorPool()
	{
		return descriptorPool;
	}

	std::vector<VkDescriptorSet> *getDescriptorSets()
	{
		return &descriptorSets;
	}

	std::vector<VkBuffer> *getUniformBuffers(){ return &uniformBuffers; }
	std::vector<VkDeviceMemory> *getUniformBuffersMemory(){ return &uniformBuffersMemory; }

private:
	VkDescriptorPool descriptorPool = {};
	std::vector<VkDescriptorSet> descriptorSets = {};
	std::vector<VkBuffer> uniformBuffers = {};
	std::vector<VkDeviceMemory> uniformBuffersMemory = {};
};

#endif // !UNIFORMDATA_DEFINED

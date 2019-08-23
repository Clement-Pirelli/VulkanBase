#pragma once
#ifndef MODEL_DEFINED
#define MODEL_DEFINED

#include "Vulkan/vulkan.h"
#include "CommandBufferData.h"
#include "UniformData.h"
#include "Renderer.h"
#include "Transform.h"
#include "Singleton.h"
#include "Utilities.h"

class Model
{
public:

	Model(Transform *givenTransform, const char *texturePath, const char *vboPath, glm::vec4 givenColor = glm::vec4(1.0f))
	{
		transform = givenTransform;
		color = givenColor;
		Renderer *renderer = Singleton<Renderer>::getInstance();
		texture = renderer->getModelTexture(texturePath);
		vbo = renderer->getModelMesh(vboPath);
		uniformDataCreationInfo uniformCreationInfo = renderer->getUniformDataCreationInfo();
		uniformCreationInfo.texture = *texture;
		uniformData = new UniformData(uniformCreationInfo);
	}

	Model(Transform *givenTransform, const char *texturePath, VertexBufferObject *givenVBO, glm::vec4 givenColor = glm::vec4(1.0f))
	{
		transform = givenTransform;
		color = givenColor;
		Renderer *renderer = Singleton<Renderer>::getInstance();
		texture = renderer->getModelTexture(texturePath);
		vbo = givenVBO;
		uniformDataCreationInfo uniformCreationInfo = renderer->getUniformDataCreationInfo();
		uniformCreationInfo.texture = *texture;
		uniformData = new UniformData(uniformCreationInfo);
	}

	~Model()
	{
		texture = nullptr;
		vbo = nullptr;
		transform = nullptr;
		delete uniformData;
	}

	void cleanupUniformData(VkDevice device)
	{
		uniformData->cleanup(device);
	}

	void onCreateCommandBuffers(commandBufferDataCreationInfo &creationInfo, VkCommandBuffer commandBuffer, unsigned int i)
	{
		VkBuffer vertexBuffers[] = { vbo->getVertexBuffer()};
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, vbo->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//uniforms
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, creationInfo.pipelineLayout, 0, 1, &uniformData->getDescriptorSets()->at(i), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vbo->getIndices().size()), 1, 0, 0, 0);
	}

	void recreateUniformData(VkDevice device, VkPhysicalDevice physicalDevice, SwapChainData *swapChainData, VkDescriptorSetLayout descriptorSetLayout)
	{
		uniformData->createData(device, physicalDevice, swapChainData, descriptorSetLayout, *texture);
	}

	UniformData *getUniformData()
	{
		return uniformData;
	}

	inline Transform *getTransform()
	{
		return transform;
	}

	VertexBufferObject *getVBO()
	{
		return vbo;
	}

	void setColor(glm::vec4 givenColor)
	{
		color = givenColor;
	}

	glm::vec4 getColor()
	{
		return color;
	}

private:
	TextureData *texture = nullptr;
	VertexBufferObject *vbo = nullptr;
	UniformData *uniformData = nullptr;
	Transform *transform = nullptr;
	glm::vec4 color;
};

#endif
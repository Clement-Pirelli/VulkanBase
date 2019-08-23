#pragma once
#ifndef COMMANDBUFFERDATA_DEFINED
#define COMMANDBUFFERDATA_DEFINED

#include "VertexBufferObject.h"
#include "UniformData.h"
#include "Renderer.h"

struct commandBufferDataCreationInfo
{
	VkDevice device;
	VkCommandPool commandPool;
	VkRenderPass renderPass;
	SwapChainData *swapChainData;
	std::vector<VkFramebuffer> *swapChainFramebuffers;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VertexBufferObject *vbo;
	UniformData *uniformData;
	Renderer *renderer;
};

class CommandBufferData
{
public:

	CommandBufferData(commandBufferDataCreationInfo &creationInfo)
	{
		createCommandBuffers(creationInfo);
	}
	CommandBufferData(){}
	void cleanup(VkDevice device, VkCommandPool commandPool)
	{
		if(set)
			vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		set = false;
	}

	void createCommandBuffers(commandBufferDataCreationInfo &creationInfo) {
		commandBuffers.resize(creationInfo.swapChainFramebuffers->size());

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = creationInfo.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.capacity();

		if (vkAllocateCommandBuffers(creationInfo.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = creationInfo.renderPass;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = creationInfo.swapChainData->extent;

		constexpr uint32_t clearValuesCount = 2;
		VkClearValue clearValues[clearValuesCount] = {};
		clearValues[0].color = { .0f, .0f, .0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValuesCount);
		renderPassInfo.pClearValues = clearValues;

		for (unsigned int i = 0; i < commandBuffers.size(); i++) {

			renderPassInfo.framebuffer = creationInfo.swapChainFramebuffers->at(i);

			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, creationInfo.graphicsPipeline);

			creationInfo.renderer->onCreateCommandBuffers(commandBuffers[i], i);			
			
			vkCmdEndRenderPass(commandBuffers[i]);

			

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
			set = true;
		}
	}

	std::vector<VkCommandBuffer> *getCommandBuffers()
	{
		return &commandBuffers;
	}

private:

	std::vector<VkCommandBuffer> commandBuffers;
	bool set = false;
};


#endif // !COMMANDBUFFERDATA_DEFINED

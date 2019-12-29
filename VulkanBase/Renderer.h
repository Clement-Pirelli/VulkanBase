#pragma once

#ifndef RENDERER_DEFINED
#define RENDERER_DEFINED

#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <Vulkan/vulkan.h>
#include "Optional.h"
#include "HandleMap.h"
#include "TextureData.h"
#include "Time.h"
#include <string>
#include "RenderDoc.h"

#pragma warning(disable: 26812)

class TextureData;
class Transform;


class Renderer {
public:

	Renderer(glm::ivec2 givenResolution, const char *givenFragmentShaderPath);

	~Renderer();
	
	void render();

	textureCreationInfo getTextureCreationInfo();

	void writeImageToFile(const char*fileName);

private:
#pragma region PRIVATE STRUCTS

	struct _QueueFamilyIndices
	{
		Optional<uint32_t> graphicsFamily;

		bool isComplete() {
			return graphicsFamily.isSet();
		}
	};

	struct ShaderData
	{
		VkPipeline graphicsPipeline = {};
		VkPipelineLayout pipelineLayout = {};

		const char* vertexPath = "";
		const char* fragmentPath = "";
	};

#pragma endregion

#pragma region VARIABLES

	const unsigned int validationLayersCount = 1;
	const char* validationLayers[1] = {
	"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;

	RenderDoc renderDocAPI;

#endif

	VkDebugUtilsMessengerEXT debugMessenger = {};

	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VkQueue graphicsQueue;

	VkQueue presentQueue;

	VkRenderPass renderPass;

	VkDescriptorSetLayout descriptorSetLayout;

	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	HandleMap<TextureData> textures;

	ShaderData shader;

	TextureData image;
	VkFramebuffer framebuffer;

	VkDescriptorPool descriptorPool = {};
	VkDescriptorSet descriptorSet = {};
	VkBuffer uniformBuffer = {};
	VkDeviceMemory uniformBufferMemory = {};

	glm::ivec2 resolution = glm::ivec2();
	
	const char* fragmentShaderPath = nullptr;
#pragma endregion

#pragma region VALIDATION_LAYERS

	bool checkValidationLayerSupport();

	void setupDebugMessenger();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

#pragma endregion

	_QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createInstance();

#pragma region DEVICES

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

	void pickPhysicalDevice();

	void createLogicalDevice();

#pragma endregion

#pragma region GRAPHICS_PIPELINE

	VkShaderModule createShaderModule(const std::vector<char>& code);

	void populateShaderData(ShaderData &shaderData);

#pragma endregion

	void createRenderPass();

	void createFramebuffers();

	void createCommandPool();

	void createSyncObjects();

	void createDescriptorSetLayout();

	void createCommandBuffer();

#pragma region TEXTURES
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

#pragma endregion

	struct screenUBO
	{
		glm::vec2 resolution = glm::vec2(.0f);
	};

	void createImage();
	void createUniformBuffer();
	void createDescriptorSet();

	void initVulkan();

	void drawFrame();

	void cleanup();

};

#endif
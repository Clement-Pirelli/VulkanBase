#pragma once

#ifndef RENDERER_DEFINED
#define RENDERER_DEFINED

#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <Vulkan/vulkan.h>
#include "Optional.h"
#include "HandleMap.h"

struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
enum VkPresentModeKHR;
struct GLFWwindow;
class TextureData;
class VertexBufferObject;
struct uniformDataCreationInfo;
struct Model;
struct SwapChainData;
struct QueueFamilyIndices;
class Transform;
class CameraComponent;
struct VBOCreationInfo;
struct textureCreationInfo;

#pragma region STRUCTS

struct QueueFamilyIndices
{
	Optional<uint32_t> graphicsFamily;
	Optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicsFamily.isSet() && presentFamily.isSet();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	unsigned int formatsCount = 0;
	VkSurfaceFormatKHR* formats = nullptr;
	unsigned int presentModesCount = 0;
	VkPresentModeKHR* presentModes = nullptr;
};

struct SwapChainData
{
	SwapChainData(){}

	VkSwapchainKHR swapChain;

	unsigned int imagesCount = 0;
	VkImage *images = nullptr;
	VkImageView *imageViews = nullptr;
	VkFormat imageFormat;
	VkExtent2D extent;

	~SwapChainData()
	{
		if (images != nullptr) delete[] images;
		if (imageViews != nullptr) delete[] imageViews;
	}
};

struct TextureHandle{ uint32_t handle; };

struct VBOHandle{ uint32_t handle; };

struct ModelHandle{ uint32_t handle; };

struct ShaderHandle{ uint32_t handle; };

struct ShaderData
{
	HandleMap<Model> modelMap;
	VkPipeline graphicsPipeline;
	VkPipelineLayout pipelineLayout;

	const char *vertexPath = "";
	const char *fragmentPath = "";
};

#pragma endregion


class Renderer {
public:

	Renderer(GLFWwindow *givenWindow);

	~Renderer();

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
	
	void render();

	void clearModelTextures();

	void clearModelVBOs();

	ModelHandle createModel(const ShaderHandle &shaderHandle, Transform *givenTransform, const char *texturePath, const char *meshPath, glm::vec4 color);
	ShaderHandle createShader(const char *fragmentShaderPath, const char *vertexShaderPath);


	uniformDataCreationInfo getUniformDataCreationInfo();

	textureCreationInfo getTextureCreationInfo();

	VBOCreationInfo getVBOCreationInfo();

	void removeModel(const ModelHandle &givenHandle, const ShaderHandle &shaderHandle);

	void setCamera(CameraComponent *givenCamera);
	void onCreateCommandBuffers(VkCommandBuffer commandBuffer, ShaderData &shader, unsigned int i);

private:

#pragma region VARIABLES

	GLFWwindow *window;

	const unsigned int validationLayersCount = 1;
	const char* validationLayers[1] = {
	"VK_LAYER_KHRONOS_validation"
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	VkDebugUtilsMessengerEXT debugMessenger;

	VkInstance instance;

	VkPhysicalDevice physicalDevice;
	VkDevice device;

	VkQueue graphicsQueue;

	VkSurfaceKHR surface;

	VkQueue presentQueue;

	SwapChainData swapChainData;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	VkRenderPass renderPass;

	VkDescriptorSetLayout descriptorSetLayout;

	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	static const unsigned int deviceExtensionsCount = 1;
	const char *deviceExtensions[deviceExtensionsCount] = { "VK_KHR_swapchain" };

	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	bool framebufferResized = false;

	HandleMap<TextureData> textures;
	HandleMap<VertexBufferObject> vbos;

	HandleMap<ShaderData> shaderMap;

	TextureData *depthTexture;

	CameraComponent *currentCamera = nullptr;
#pragma endregion

#pragma region VALIDATION_LAYERS

	bool checkValidationLayerSupport();

	const char** getRequiredExtensions(unsigned int &count);

	void setupDebugMessenger();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

#pragma endregion

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, const unsigned int availablePressentModesCount);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, const unsigned int availableFormatsCount);

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createInstance();

#pragma region DEVICES

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

	void pickPhysicalDevice();

	void createLogicalDevice();

#pragma endregion

	void createSurface();

#pragma region SWAP_CHAIN

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	void createSwapChain();

#pragma endregion

	void createImageViews();

#pragma region GRAPHICS_PIPELINE
	VkShaderModule createShaderModule(const std::vector<char>& code);

	void populateShaderData(ShaderData &shaderData);

#pragma endregion

	void createRenderPass();

	void createFramebuffers();

	void createCommandPool();

	void createSyncObjects();

	void cleanupSwapChain();

	void recreateSwapChain();

	void createDescriptorSetLayout();

	void updateUniformBuffer(uint32_t currentImage);

	void recreateCommandBufferData();

	void createCommandBuffers();

#pragma region DEPTH_TEXTURE
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	void createDepthResources();

#pragma endregion


	void initVulkan();

	void drawFrame();

	void cleanup();

};

#endif
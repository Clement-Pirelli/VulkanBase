#pragma once

#ifndef RENDERER_DEFINED
#define RENDERER_DEFINED

#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <Vulkan/vulkan.h>

struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
enum VkPresentModeKHR;
struct GLFWwindow;
class TextureData;
class VertexBufferObject;
struct uniformDataCreationInfo;
struct commandBufferDataCreationInfo;
class Model;
struct SwapChainData;
struct QueueFamilyIndices;
class Transform;
class CameraComponent;
class CommandBufferData;
struct VBOCreationInfo;

#pragma region STRUCTS


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

#pragma endregion


class Renderer {
public:

	Renderer(GLFWwindow *givenWindow);

	~Renderer();

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
	
	void render();

	TextureData *getModelTexture(const char* path);

	void clearModelTextures();

	void clearModelVBOs();

	VertexBufferObject *getModelMesh(const char *path);

	commandBufferDataCreationInfo getCommandBufferDataCreationInfo();

	uniformDataCreationInfo getUniformDataCreationInfo();

	VBOCreationInfo getVBOCreationInfo();

	void addModel(Model *givenModel);
	void removeModel(Model *givenModel);
	void addUIModel(Model *givenUIModel);
	void removeUIModel(Model *givenUIModel);

	void setCamera(CameraComponent *givenCamera);
	void onCreateCommandBuffers(VkCommandBuffer commandBuffer, unsigned int i);

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
	VkPipelineLayout pipelineLayout;


	VkPipeline graphicsPipeline;

	VkCommandPool commandPool;
	CommandBufferData *commandBufferData = nullptr;


	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	static const unsigned int deviceExtensionsCount = 1;
	const char *deviceExtensions[deviceExtensionsCount] = { "VK_KHR_swapchain" };

	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;
	bool framebufferResized = false;


	std::unordered_map<std::string, TextureData*> textures;
	std::unordered_map<std::string, VertexBufferObject*> vbos;
	std::vector<Model*> models;
	std::vector<Model*> UIModels;

	TextureData *depthTexture;

	CameraComponent *currentCamera;
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

	void createGraphicsPipeline();

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
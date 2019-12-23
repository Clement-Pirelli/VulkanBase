#pragma once

#ifndef RENDERER_DEFINED
#define RENDERER_DEFINED

#include <glm.hpp>
#include <vector>
#include <unordered_map>
#include <Vulkan/vulkan.h>
#include "Optional.h"
#include "HandleMap.h"
#include "Camera.h"
#include "TextureData.h"
#pragma warning(disable: 26812)

struct VkSurfaceCapabilitiesKHR;
struct VkSurfaceFormatKHR;
enum VkPresentModeKHR;
struct GLFWwindow;
class TextureData;
class VertexBufferObject;
struct uniformDataCreationInfo;
struct Model;
class Transform;
struct VBOCreationInfo;
struct textureCreationInfo;

#pragma region PUBLIC STRUCTS

struct SwapChainData
{
	SwapChainData(){}

	VkSwapchainKHR swapChain;

	unsigned int imagesCount = 0;
	VkImage *images = nullptr;
	VkImageView *imageViews = nullptr;
	VkFormat imageFormat = {};
	VkExtent2D extent = {};

	~SwapChainData()
	{
		if (images != nullptr) delete[] images;
		if (imageViews != nullptr) delete[] imageViews;
	}
};


template<typename T> 
bool isHandleValid(const T &handle)
{
	return handle.handle != invalidHandle;
}

struct TextureHandle{ uint32_t handle; };

struct VBOHandle{ uint32_t handle; };

struct ShaderHandle{ uint32_t handle; };

struct ModelHandle{
	uint32_t handle;
	ShaderHandle shaderHandle;
};

struct PointLight
{
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

struct DirLight
{
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;
};

struct PointLightHandle { uint32_t handle; };

struct DirLightHandle { uint32_t handle; };

#pragma endregion


class Renderer {
public:

	Renderer(GLFWwindow *givenWindow, glm::vec2 givenResolution);

	~Renderer();

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
	
	void render();

	void clearModelTextures();

	void clearModelVBOs();

	ModelHandle createModel(const ShaderHandle &shaderHandle, const Transform &givenTransform, const char *texturePath, const char *meshPath, glm::vec4 color);
	Model &getModel(const ModelHandle &handle);
	std::vector<ModelHandle> createModels(const ShaderHandle &shaderHandle, std::vector<Transform> &givenTransforms, const char *texturePath, const char *meshPath, std::vector<glm::vec4> colors);
	ShaderHandle createShader(const char *fragmentShaderPath, const char *vertexShaderPath);
	
	PointLightHandle createPointLight(glm::vec3 position, glm::vec3 color, float intensity);
	DirLightHandle createDirLight(glm::vec3 direction, glm::vec3 color, float intensity);
	PointLight &getPointLight(PointLightHandle handle);
	DirLight &getDirLight(DirLightHandle handle);

	void destroyPointLight(PointLightHandle handle);
	void destroyDirLight(DirLightHandle handle);

	uniformDataCreationInfo getUniformDataCreationInfo();
	textureCreationInfo getTextureCreationInfo();
	VBOCreationInfo getVBOCreationInfo();

	void destroyModel(const ModelHandle &givenHandle);
	
	Camera &getCamera();

private:
#pragma region PRIVATE STRUCTS

	struct _QueueFamilyIndices
	{
		Optional<uint32_t> graphicsFamily;
		Optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.isSet() && presentFamily.isSet();
		}
	};


	struct _SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		unsigned int formatsCount = 0;
		VkSurfaceFormatKHR* formats = nullptr;
		unsigned int presentModesCount = 0;
		VkPresentModeKHR* presentModes = nullptr;
	};

	struct ShaderData
	{
		HandleMap<Model> modelMap = {};
		VkPipeline graphicsPipeline = {};
		VkPipelineLayout pipelineLayout = {};

		const char* vertexPath = "";
		const char* fragmentPath = "";
	};

#pragma endregion

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

	HandleMap<ShaderData> shaderMap;

	HandleMap<PointLight> pointLMap;
	HandleMap<DirLight> dirLMap;

	TextureData depthTexture;
	TextureData normalTexture;

	Camera camera;
	glm::vec2 resolution = glm::vec2();
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

	_QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createInstance();

#pragma region DEVICES

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool isPhysicalDeviceSuitable(VkPhysicalDevice device);

	void pickPhysicalDevice();

	void createLogicalDevice();

#pragma endregion

	void createSurface();

#pragma region SWAP_CHAIN

	_SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

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
#pragma region NORMAL_TEXTURE
	void createNormalResources();
#pragma endregion


	void onCreateCommandBuffers(VkCommandBuffer commandBuffer, ShaderData &shader, unsigned int i);

	void initVulkan();

	void drawFrame();

	void cleanup();

};

#endif
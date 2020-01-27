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
#include "Time.h"
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


constexpr unsigned int maxDirLights = 10;
constexpr unsigned int maxPointLights = 100;
constexpr unsigned int ssaoKernelSize = 32;

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

	ModelHandle createModel(const Transform &givenTransform, const char *texturePath, const char *meshPath, glm::vec4 color);
	Model &getModel(const ModelHandle &handle);
	std::vector<ModelHandle> createModels(std::vector<Transform> &givenTransforms, const char *texturePath, const char *meshPath, std::vector<glm::vec4> colors);
	
	
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
	
	void toggleSSAO();

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
	"VK_LAYER_LUNARG_standard_validation"
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
	ShaderHandle shaderHandle = {};

	HandleMap<PointLight> pointLMap;
	HandleMap<DirLight> dirLMap;


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

	ShaderHandle createShader(const char* fragmentShaderPath, const char* vertexShaderPath);

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

	void updateUniformBuffers(uint32_t currentImage);

	void recreateCommandBufferData();

	void createCommandBuffers();

#pragma region TEXTURES
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	VkFormat findDepthFormat();

	void createGBufferResources();

#pragma endregion

#pragma region DEFERRED
	struct DeferredResources
	{
		std::vector<std::pair<glm::vec3, glm::vec2>> quadVertices = {};
		VkPipeline pipeline = {};
		VkPipelineLayout pipelineLayout = {};
		VkRenderPass renderPass = {};
		std::vector<VkFramebuffer> frameBuffers = {};
		VkDescriptorSetLayout descriptorSetLayout = {};
		std::vector<VkDescriptorSet> descriptorSets = {};
		VkDescriptorPool descriptorPool = {};
		std::vector<VkBuffer> uniformBuffers = {};
		std::vector<VkDeviceMemory> uniformBuffersMemory = {};
	} deferredResources;

	struct GBuffer
	{
		TextureData colorTexture;
		TextureData depthTexture;
		TextureData normalTexture;
		TextureData positionTexture;
		VkSampler sampler;
	} gBuffer;


	glm::vec4 ssaoKernel[ssaoKernelSize] = {};
	bool ssaoOn = true;

	void setSSAOKernel();

	struct DeferredUBO
	{
		float time = Time::now().asSeconds();
		int dirLightAmount = 0;
		int pointLightAmount = 0;
		int ssaoOn = 1;
		glm::mat4 projection = {};
		glm::vec4 cameraPosition = glm::vec4(.0f);			
		glm::vec4 dirLightsDirections[maxDirLights] = {};	
		glm::vec4 pointLightsPositions[maxPointLights] = {};
		 //color also encodes intensity (w)
		glm::vec4 dirLightsColors[maxDirLights] = {};		
		glm::vec4 pointLightsColors[maxPointLights] = {};
		glm::vec4 ssaoKernel[ssaoKernelSize] = {};
		glm::vec2 mouse = glm::vec2(.0f);					
		glm::vec2 resolution = glm::vec2(.0f);

		void setDirLight(int index, glm::vec3 color, float intensity, glm::vec3 direction)
		{
#ifndef NDEBUG
			if (index > maxDirLights) return;
#endif
			dirLightsColors[index] = glm::vec4(color, intensity);
			dirLightsDirections[index] = glm::vec4(direction, .0f);
		}

		void setPointLight(int index, glm::vec3 color, float intensity, glm::vec3 position)
		{
#ifndef NDEBUG
			if (index > maxPointLights) return;
#endif
			pointLightsColors[index] = glm::vec4(color, intensity);
			pointLightsPositions[index] = glm::vec4(position, 1.0f);
		}
	};




	void populateGBufferResource(TextureData& resource, VkFormat format, VkImageAspectFlagBits aspectFlagBits, VkImageUsageFlagBits usageFlagBits, VkImageLayout endLayout);

	void createDeferredShader();

	void createDeferredUniformBuffers();

	void createDeferredDescriptorSets();

#pragma endregion

	void onCreateCommandBuffers(VkCommandBuffer commandBuffer, ShaderData &shader, unsigned int i);

	void initVulkan();

	void drawFrame();

	void cleanup();

};

#endif
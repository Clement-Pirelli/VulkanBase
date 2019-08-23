#include "Renderer.h"

#define GLFW_VULKAN_DEFINED
#include <glfw3.h>
#include <stdexcept>

#include "Vertex.h"
#include "QueueFamilyIndices.h"
#include "VulkanUtilities.h"
#include "UniformData.h"
#include "CommandBufferData.h"
#include "VertexBufferObject.h"
#include "UniformData.h"
#include "TextureData.h"
#include "Model.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "Entity.h"

#pragma region PUBLIC

Renderer::Renderer(GLFWwindow *givenWindow){
	window = givenWindow;
	physicalDevice = VK_NULL_HANDLE;
	initVulkan();
}

Renderer::~Renderer()
{
	vkDeviceWaitIdle(device);
	cleanup();
}

void Renderer::render()
{
	drawFrame();
}

TextureData *Renderer::getModelTexture(const char* path)
{
	std::string pathStr = std::string(path);
	//if a texture already exists at that path, return that instead of creating a duplicate
	if (textures.count(path) > 0)
		return textures.at(path);


	textureCreationInfo creationInfo;
	creationInfo.device = device;
	creationInfo.physicalDevice = physicalDevice;
	creationInfo.graphicsQueue = graphicsQueue;
	creationInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	creationInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	creationInfo.path = path;
	creationInfo.hasSampler = true;
	creationInfo.commandPool = commandPool;
	TextureData *returnData = new TextureData(creationInfo);

	textures.insert(std::pair<std::string, TextureData*>(pathStr, returnData));

	return returnData;
}

void Renderer::clearModelTextures()
{
	for (auto textureData : textures)
	{
		textureData.second->cleanup(device);
		delete textureData.second;
	}

	textures.clear();
}

VertexBufferObject *Renderer::getModelMesh(const char *path)
{
	std::string pathStr = std::string(path);
	if (vbos.count(pathStr) > 0)
		return vbos.at(path);

	VBOCreationInfo creationInfo = getVBOCreationInfo();
	VertexBufferObject *returnVBO = new VertexBufferObject(creationInfo, path);

	vbos.insert(std::pair<std::string, VertexBufferObject*>(pathStr,returnVBO));

	return returnVBO;
}

void Renderer::clearModelVBOs()
{
	for (auto vboData : vbos)
	{
		vboData.second->cleanup(device);
		delete vboData.second;
	}
	vbos.clear();
}

commandBufferDataCreationInfo Renderer::getCommandBufferDataCreationInfo()
{
	commandBufferDataCreationInfo returnInfo;
	returnInfo.device = device;
	returnInfo.pipelineLayout = pipelineLayout;
	returnInfo.swapChainData = &swapChainData;
	returnInfo.swapChainFramebuffers = &swapChainFramebuffers;
	returnInfo.commandPool = commandPool;
	returnInfo.renderPass = renderPass;
	returnInfo.graphicsPipeline = graphicsPipeline;
	returnInfo.renderer = this;
	return returnInfo;
}

uniformDataCreationInfo Renderer::getUniformDataCreationInfo()
{
	uniformDataCreationInfo returnInfo;
	returnInfo.device = device;
	returnInfo.descriptorSetLayout = descriptorSetLayout;
	returnInfo.physicalDevice = physicalDevice;
	returnInfo.swapChainData = &swapChainData;
	return returnInfo;
}

VBOCreationInfo Renderer::getVBOCreationInfo()
{
	VBOCreationInfo creationInfo;
	creationInfo.commandPool = commandPool;
	creationInfo.device = device;
	creationInfo.graphicsQueue = graphicsQueue;
	creationInfo.physicalDevice = physicalDevice;
	return creationInfo;
}

void Renderer::addModel(Model * givenModel)
{
	models.push_back(givenModel);
	recreateCommandBufferData();
}

void Renderer::removeModel(Model * givenModel)
{
	vkDeviceWaitIdle(device);

	auto it = models.end();
	for (unsigned int i = 0; i < models.size(); i++)
	{
		if (models[i] == givenModel)
		{
			it = models.begin() + i;
		}
	}

	if (it != models.end())
	{
		Model *m = *it;
		m->cleanupUniformData(device);
		models.erase(it);
	}

	recreateCommandBufferData();
}
void Renderer::addUIModel(Model *givenUIModel)
{
	UIModels.push_back(givenUIModel);
	recreateCommandBufferData();
}

void Renderer::removeUIModel(Model * givenUIModel)
{
	vkDeviceWaitIdle(device);

	auto it = UIModels.end();
	for (unsigned int i = 0; i < UIModels.size(); i++)
	{
		if (UIModels[i] == givenUIModel)
		{
			it = UIModels.begin() + i;
		}
	}

	if (it != UIModels.end())
	{
		Model *m = *it;
		m->cleanupUniformData(device);
		UIModels.erase(it);
	}

	recreateCommandBufferData();
}

void Renderer::setCamera(CameraComponent *givenCamera)
{
	currentCamera = givenCamera;
}

#pragma endregion

bool Renderer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers;
	availableLayers.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (unsigned int i = 0; i < validationLayersCount; i++) {
		bool layerFound = false;

		for (unsigned int j = 0; j < availableLayers.size(); j++)
		{
			if (strcmp(availableLayers[j].layerName, validationLayers[i]) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

const char** Renderer::getRequiredExtensions(unsigned int &count)
{
	uint32_t glfwExtensionCount = 0;
	glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	if (enableValidationLayers) glfwExtensionCount++;

	const uint32_t arraySize = glfwExtensionCount;
	const char **extensions;
	extensions = new const char *[glfwExtensionCount];

	uint32_t a;
	extensions = glfwGetRequiredInstanceExtensions(&a);

	if (enableValidationLayers) {
		extensions[glfwExtensionCount - 1] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	}
	count = glfwExtensionCount;
	return extensions;
}

void Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		VkExtent2D actualExtent = { (uint32_t)width, (uint32_t)height };

		actualExtent.width = max(capabilities.minImageExtent.width, min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = max(capabilities.minImageExtent.height, min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}

}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, const unsigned int availablePressentModesCount)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (unsigned int i = 0; i < availablePressentModesCount; i++) {
		VkPresentModeKHR availablePresentMode = availablePresentModes[i];
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		} else if (availablePresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const VkSurfaceFormatKHR* availableFormats, const unsigned int availableFormatsCount)
{
	for (unsigned int i = 0; i < availableFormatsCount; i++) {
		VkSurfaceFormatKHR availableFormat = availableFormats[i];
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	VkQueueFamilyProperties *queueFamilies = new VkQueueFamilyProperties[queueFamilyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	for (unsigned int i = 0; i < queueFamilyCount; i++) {
		VkQueueFamilyProperties queueFamily = queueFamilies[i];

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily.setValue(i);
		}

		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily.setValue(i);
			break;
		}
	}

	delete[] queueFamilies;

	return indices;
}

void Renderer::createInstance() {

	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}


	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersCount);
		createInfo.ppEnabledLayerNames = validationLayers;

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	const char **glfwExtensions = nullptr;
	unsigned int glfwExtensionsCount;
	glfwExtensions = getRequiredExtensions(glfwExtensionsCount);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensionsCount);
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance!");
	}

}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {

	uint32_t availableExtensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, nullptr);

	VkExtensionProperties* availableExtensions = new VkExtensionProperties[availableExtensionCount];
	vkEnumerateDeviceExtensionProperties(device, nullptr, &availableExtensionCount, availableExtensions);

	std::vector<std::string> requiredExtensions;
	for (unsigned int i = 0; i < deviceExtensionsCount; i++)
	{
		requiredExtensions.push_back(deviceExtensions[i]);
	}

	for (unsigned int i = 0; i < deviceExtensionsCount; i++)
	{
		for (unsigned int j = 0; j < availableExtensionCount; j++)
		{
			if (requiredExtensions[i] == availableExtensions->extensionName)
			{
				delete[] availableExtensions;
				return true;
			}
		}
	}

	delete[] availableExtensions;
	return false;
}

bool Renderer::isPhysicalDeviceSuitable(VkPhysicalDevice device) 
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = swapChainSupport.formatsCount != 0 && swapChainSupport.presentModesCount != 0;
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

void Renderer::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	VkPhysicalDevice *devices = new VkPhysicalDevice[deviceCount];
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

	for (unsigned int i = 0; i < deviceCount; i++) {
		VkPhysicalDevice dev = devices[i];
		if (isPhysicalDeviceSuitable(dev)) {
			physicalDevice = dev;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	delete[] devices;
}

void Renderer::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);


	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.getValue(), indices.presentFamily.getValue() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};

		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensionsCount);
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayersCount);
		createInfo.ppEnabledLayerNames = validationLayers;
	} else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, indices.graphicsFamily.getValue(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.getValue(), 0, &presentQueue);
}

void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}
}

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device) 
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formatsCount = formatCount;
		details.formats = new VkSurfaceFormatKHR[formatCount];
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats);
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModesCount = presentModeCount;
		details.presentModes = new VkPresentModeKHR[presentModeCount];
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes);
	}

	return details;
}

void Renderer::createSwapChain() 
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats, swapChainSupport.formatsCount);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes, swapChainSupport.presentModesCount);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = { indices.graphicsFamily.getValue(), indices.presentFamily.getValue() };

	if (indices.graphicsFamily.getValue() != indices.presentFamily.getValue()) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChainData.swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}


	vkGetSwapchainImagesKHR(device, swapChainData.swapChain, &imageCount, nullptr);
	swapChainData.imagesCount = imageCount;
	swapChainData.images = new VkImage[imageCount];
	vkGetSwapchainImagesKHR(device, swapChainData.swapChain, &imageCount, swapChainData.images);

	swapChainData.imageFormat = surfaceFormat.format;
	swapChainData.extent = extent;
}

void Renderer::createImageViews()
{
	swapChainData.imageViews = new VkImageView[swapChainData.imagesCount];
	VkFormat format = swapChainData.imageFormat;
	for (size_t i = 0; i < swapChainData.imagesCount; i++)
	{
		swapChainData.imageViews[i] = createImageView(device, swapChainData.images[i], format, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

void Renderer::createGraphicsPipeline(){
	std::vector<char> vertShaderCode;
	readFile("shaders/vert.spv", &vertShaderCode);
	std::vector<char> fragShaderCode;
	readFile("shaders/frag.spv", &fragShaderCode);

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
	unsigned int attributeDescriptionsCount;
	VkVertexInputAttributeDescription *attributeDescriptions = Vertex::getAttributeDescriptions(attributeDescriptionsCount);

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptionsCount);
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainData.extent.width;
	viewport.height = (float)swapChainData.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainData.extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	//rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	//rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional


	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	delete[] vertexInputInfo.pVertexAttributeDescriptions;

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Renderer::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainData.imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	constexpr uint32_t attachmentsCount = 2;
	VkAttachmentDescription attachments[attachmentsCount] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsCount);
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void Renderer::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainData.imagesCount);
	constexpr uint32_t attachmentsCount = 2;
	for (size_t i = 0; i < swapChainData.imagesCount; i++) {

		VkImageView attachments[attachmentsCount] = {
			swapChainData.imageViews[i],
			depthTexture->getView()
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentsCount;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainData.extent.width;
		framebufferInfo.height = swapChainData.extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void Renderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.getValue();
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void Renderer::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores.data()[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores.data()[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences.data()[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create semaphores!");
		}
	}
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
}

void Renderer::cleanupSwapChain()
{
	depthTexture->cleanup(device);
	delete depthTexture;

	for (unsigned int i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	commandBufferData->cleanup(device, commandPool);

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (unsigned int i = 0; i < swapChainData.imagesCount; i++)
	{
		vkDestroyImageView(device, swapChainData.imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChainData.swapChain, nullptr);

	for(Model *m : models)
	{
		m->cleanupUniformData(device);
	}
	for(Model* m : UIModels)
	{
		m->cleanupUniformData(device);
	}
}

void Renderer::recreateSwapChain(){

	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResources();
	createFramebuffers();

	for (Model *m : models)
	{
		m->recreateUniformData(device, physicalDevice, &swapChainData, descriptorSetLayout);
	}
	for(Model *m : UIModels)
	{
		m->recreateUniformData(device, physicalDevice, &swapChainData, descriptorSetLayout);
	}
	commandBufferDataCreationInfo creationInfo = getCommandBufferDataCreationInfo();
	commandBufferData->createCommandBuffers(creationInfo);
}

void Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[2] = { uboLayoutBinding, samplerLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = bindings;

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

void Renderer::updateUniformBuffer(uint32_t currentImage)
{
	glm::mat4 viewMat = glm::mat4(1.0f);
	glm::mat4 projMat = glm::mat4(1.0f);
	glm::mat4 modelMat = glm::mat4(1.0f);

	UniformBufferObject ubo = {};
	if (currentCamera != nullptr)
	{
		viewMat = currentCamera->getOwner()->getTransform()->getGlobalTransform();
		projMat = glm::perspective(glm::radians(currentCamera->getDegFOV()), swapChainData.extent.width / (float)swapChainData.extent.height, currentCamera->getZNear(), currentCamera->getZFar());
		
		//opengl to vulkan : y axis is inverted. Image will be rendered upside down if we don't do this
		projMat[1][1] *= -1;
	}

	for (Model *m : models)
	{
		modelMat = m->getTransform()->getGlobalTransform();
		ubo.world = projMat * viewMat * modelMat;
		ubo.color = m->getColor();
		void* data;
		vkMapMemory(device, m->getUniformData()->getUniformBuffersMemory()->at(currentImage), 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, m->getUniformData()->getUniformBuffersMemory()->at(currentImage));
	}

	viewMat = glm::mat4(1.0f);
	projMat = glm::ortho(-1.0f, 1.0f, 1.0f, -1.0f);

	for(Model* m : UIModels)
	{		
		modelMat = m->getTransform()->getLocalTransform();

		ubo.world = projMat * viewMat * modelMat;
		ubo.color = m->getColor();

		void* data;
		vkMapMemory(device, m->getUniformData()->getUniformBuffersMemory()->at(currentImage), 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, m->getUniformData()->getUniformBuffersMemory()->at(currentImage));
	}
}

void Renderer::recreateCommandBufferData()
{
	vkDeviceWaitIdle(device);
	commandBufferDataCreationInfo creationInfo = getCommandBufferDataCreationInfo();
	if(commandBufferData == nullptr)
	{
		commandBufferData = new CommandBufferData(creationInfo);
	}
	else 
	{
		commandBufferData->cleanup(device, commandPool);
		commandBufferData->createCommandBuffers(creationInfo);
	}
}

void Renderer::onCreateCommandBuffers(VkCommandBuffer commandBuffer, unsigned int i)
{
	commandBufferDataCreationInfo creationInfo = getCommandBufferDataCreationInfo();
	for(Model *m : models)
	{
		m->onCreateCommandBuffers(creationInfo, commandBuffer, i);
	}
	for(Model *m : UIModels)
	{
		m->onCreateCommandBuffers(creationInfo, commandBuffer, i);
	}
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (unsigned int i = 0; i < candidates.size(); i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidates[i];
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return candidates[i];
		}

		throw std::runtime_error("failed to find supported format!");
	}

	return VkFormat::VK_FORMAT_UNDEFINED;
}

VkFormat Renderer::findDepthFormat() {
	return findSupportedFormat(
		std::vector<VkFormat> { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void Renderer::createDepthResources()
{
	VkFormat depthFormat = findDepthFormat();
	depthTexture = new TextureData();
	depthTexture->createImage(
		device,
		physicalDevice,
		swapChainData.extent.width,
		swapChainData.extent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	depthTexture->setView(createImageView(device, depthTexture->getImage(), depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT));
	TextureData::transitionImageLayout(device, commandPool, graphicsQueue, depthTexture->getImage(), depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void Renderer::initVulkan() {
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResources();
	createFramebuffers();
	createSyncObjects();
}

void Renderer::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	VkSemaphore currentSemaphore = imageAvailableSemaphores[currentFrame];

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		device,
		swapChainData.swapChain,
		std::numeric_limits<uint64_t>::max(),
		currentSemaphore,
		VK_NULL_HANDLE,
		&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		recreateSwapChain(); return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR){ throw std::runtime_error("failed to acquire swap chain image!"); }

	updateUniformBuffer(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { currentSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	
	submitInfo.pCommandBuffers = &commandBufferData->getCommandBuffers()->at(imageIndex);

	VkSemaphore signalSemaphores[1] = { currentSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkFence *toReset = &inFlightFences[currentFrame];
	vkResetFences(device, 1, toReset);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, *toReset) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[1] = { swapChainData.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized)
	{
		framebufferResized = false;
		recreateSwapChain();
	} else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::cleanup() 
{
	cleanupSwapChain();

	clearModelTextures();

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

	clearModelVBOs();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	vkDestroySurfaceKHR(instance, surface, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroyInstance(instance, nullptr);
}

void Renderer::framebufferResizeCallback(GLFWwindow * window, int width, int height)
{
	Renderer *ren = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
	ren->framebufferResized = true;
}
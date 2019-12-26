#include "Renderer.h"

#define GLFW_VULKAN_DEFINED
#include <glfw3.h>
#include <stdexcept>

#include "Vertex.h"
#include "VulkanUtilities.h"
#include "UniformData.h"
#include "VertexBufferObject.h"
#include "Model.h"
#include "Transform.h"
#include "Camera.h"
#include "ResourceProvider.h"
#include "InputManager.h"

#pragma warning(disable: 6385)
#pragma warning(disable: 26812)

#pragma region PUBLIC

Renderer::Renderer(GLFWwindow *givenWindow, glm::vec2 givenResolution) : camera(Transform(), .0f, .0f, .0f,.0f), resolution(givenResolution) {
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

void Renderer::clearModelTextures()
{
	textures.clear();
	ResourceProvider<TextureData>::clearResources();
}

ModelHandle Renderer::createModel(const Transform &givenTransform, const char *texturePath, const char *meshPath, glm::vec4 color = glm::vec4(1.0f,1.0f,1.0f,1.0f))
{
	TextureData &textureData = ResourceProvider<TextureData>::getResource(std::string(texturePath));
	VertexBufferObject &vbo = ResourceProvider<VertexBufferObject>::getResource(std::string(meshPath));
	TextureHandle textureHandle = { textures.insert(textureData) };
	Model m(&vbo);
	m.textureHandle = textureHandle;
	m.color = color;
	uniformDataCreationInfo creationInfo = getUniformDataCreationInfo();
	creationInfo.texture = textureData;
	m.uniformData = UniformData(creationInfo);
	m.transform = givenTransform;

	ShaderData &shader = shaderMap[shaderHandle.handle];
	uint32_t handle = shader.modelMap.insert(m);

	recreateCommandBufferData();

	return ModelHandle{ handle, shaderHandle };
}

Model &Renderer::getModel(const ModelHandle & handle)
{
	return shaderMap[handle.shaderHandle.handle].modelMap[handle.handle];
}

std::vector<ModelHandle> Renderer::createModels(std::vector<Transform> &givenTransforms, const char * texturePath, const char * meshPath, std::vector<glm::vec4> colors)
{
	assert(givenTransforms.size() == colors.size());
	const size_t modelAmount = givenTransforms.size();


	TextureData &textureData = ResourceProvider<TextureData>::getResource(std::string(texturePath));
	VertexBufferObject &vbo = ResourceProvider<VertexBufferObject>::getResource(std::string(meshPath));
	TextureHandle textureHandle = { textures.insert(textureData) };
	ShaderData &shader = shaderMap[shaderHandle.handle];

	std::vector<ModelHandle> handles;
	handles.resize(modelAmount);

	Model m(&vbo);
	m.textureHandle = textureHandle;
	uniformDataCreationInfo creationInfo = getUniformDataCreationInfo();
	creationInfo.texture = textureData;

	for(size_t i = 0; i < modelAmount; i++)
	{
		m.color = colors[i];
		m.uniformData = UniformData(creationInfo);
		m.transform = givenTransforms[i];
		uint32_t handle = shader.modelMap.insert(m);
		handles[i] = ModelHandle{ handle, shaderHandle };
	}

	recreateCommandBufferData();

	return handles;
}

ShaderHandle Renderer::createShader(const char * fragmentShaderPath, const char * vertexShaderPath)
{
	ShaderData shaderData;
	shaderData.fragmentPath = fragmentShaderPath;
	shaderData.vertexPath = vertexShaderPath;
	populateShaderData(shaderData);

	uint32_t handle = shaderMap.insert(shaderData);
	return ShaderHandle{ handle };
}

PointLightHandle Renderer::createPointLight(glm::vec3 position, glm::vec3 color, float intensity)
{
	if (pointLMap.count() >= maxPointLights) return PointLightHandle{ invalidHandle };
	PointLight light = { position, color, intensity };
	uint32_t handle = pointLMap.insert(light);
	return PointLightHandle{ handle };
}

DirLightHandle Renderer::createDirLight(glm::vec3 direction, glm::vec3 color, float intensity)
{
	if (dirLMap.count() >= maxPointLights) return DirLightHandle{ invalidHandle };
	DirLight light = { direction, color, intensity };
	uint32_t handle = dirLMap.insert(light);
	return DirLightHandle{ handle };
}

PointLight& Renderer::getPointLight(PointLightHandle handle)
{
	return pointLMap[handle.handle];
}

DirLight& Renderer::getDirLight(DirLightHandle handle)
{
	return dirLMap[handle.handle];
}

void Renderer::destroyPointLight(PointLightHandle handle)
{
	pointLMap.remove(handle.handle);
}

void Renderer::destroyDirLight(DirLightHandle handle)
{
	dirLMap.remove(handle.handle);
}

void Renderer::clearModelVBOs()
{
	ResourceProvider<VertexBufferObject>::clearResources();
}

uniformDataCreationInfo Renderer::getUniformDataCreationInfo()
{
	uniformDataCreationInfo returnInfo(swapChainData);
	returnInfo.device = device;
	returnInfo.descriptorSetLayout = descriptorSetLayout;
	returnInfo.physicalDevice = physicalDevice;
	return returnInfo;
}

textureCreationInfo Renderer::getTextureCreationInfo()
{
	textureCreationInfo creationInfo;
	creationInfo.device = device;
	creationInfo.physicalDevice = physicalDevice;
	creationInfo.graphicsQueue = graphicsQueue;
	creationInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	creationInfo.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
	creationInfo.hasSampler = true;
	creationInfo.commandPool = commandPool;
	return creationInfo;
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

void Renderer::destroyModel(const ModelHandle &givenHandle)
{
	vkDeviceWaitIdle(device);

	ShaderData &shader = shaderMap[givenHandle.shaderHandle.handle];
	Model model = shader.modelMap[givenHandle.handle];
	model.uniformData.cleanup(device);
	shader.modelMap.remove(givenHandle.handle);

	recreateCommandBufferData();
}

Camera &Renderer::getCamera()
{
	return camera;
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


VkPresentModeKHR 
#pragma warning(suppress: 26812)
Renderer::chooseSwapPresentMode(const VkPresentModeKHR* availablePresentModes, const unsigned int availablePressentModesCount)
{
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (unsigned int i = 0; i < availablePressentModesCount; i++) {
		VkPresentModeKHR availablePresentMode = availablePresentModes[i];
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		} else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
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

Renderer::_QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device) {
	_QueueFamilyIndices indices;

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
	appInfo.pApplicationName = "Bwaa";
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
			if (requiredExtensions[i] == availableExtensions[j].extensionName)
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
	_QueueFamilyIndices indices = findQueueFamilies(device);
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		_SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
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
	_QueueFamilyIndices indices = findQueueFamilies(physicalDevice);


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

Renderer::_SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device) 
{
	_SwapChainSupportDetails details;

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
	_SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

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

	_QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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
		swapChainData.imageViews[i] = createImageView(device, swapChainData.images[i], format, VK_IMAGE_ASPECT_COLOR_BIT, 1);
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

void Renderer::populateShaderData(ShaderData& shaderData) {
	std::vector<char> vertShaderCode;
	readFile(shaderData.vertexPath, vertShaderCode);
	std::vector<char> fragShaderCode;
	readFile(shaderData.fragmentPath, fragShaderCode);

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
	VkVertexInputAttributeDescription* attributeDescriptions = Vertex::getAttributeDescriptions(attributeDescriptionsCount);

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
	colorBlending.attachmentCount = 3;

	VkPipelineColorBlendAttachmentState colorBlendAttachments[3] = { colorBlendAttachment ,colorBlendAttachment, colorBlendAttachment };
	colorBlending.pAttachments = colorBlendAttachments;
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

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &shaderData.pipelineLayout) != VK_SUCCESS) {
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
	pipelineInfo.layout = shaderData.pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional


	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &shaderData.graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	delete[] vertexInputInfo.pVertexAttributeDescriptions;

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Renderer::createRenderPass()
{
	//geometry pass
	{
		VkAttachmentDescription colorAttachment = {};
		{
			colorAttachment.format = swapChainData.imageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}


		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment = {};
		{
			depthAttachment.format = findDepthFormat();
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}


		VkAttachmentReference depthAttachmentRef = {};
		depthAttachmentRef.attachment = 2;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription normalAttachment = {};
		{
			normalAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			normalAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			normalAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			normalAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			normalAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			normalAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			normalAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			normalAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}


		VkAttachmentReference normalAttachmentRef = {};
		normalAttachmentRef.attachment = 1;
		normalAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription positionAttachment = {};
		{
			positionAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
			positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			positionAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			positionAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			positionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}


		VkAttachmentReference positionAttachmentRef = {};
		positionAttachmentRef.attachment = 3;
		positionAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 3;
		VkAttachmentReference refs[3] = { colorAttachmentRef, normalAttachmentRef, positionAttachmentRef };
		subpass.pColorAttachments = refs;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		constexpr uint32_t attachmentsCount = 4;
		VkAttachmentDescription attachments[attachmentsCount] = { colorAttachment, normalAttachment, depthAttachment, positionAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = attachmentsCount;
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

	//ssao pass
	{
		VkAttachmentDescription colorAttachment = {};
		{
			colorAttachment.format = swapChainData.imageFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		}

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		constexpr uint32_t attachmentsCount = 1;
		VkAttachmentDescription attachments[attachmentsCount] = { colorAttachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentsCount);
		renderPassInfo.pAttachments = attachments;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &deferredResources.renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
	}

}

void Renderer::createFramebuffers() {
	swapChainFramebuffers.resize(swapChainData.imagesCount);
	deferredResources.frameBuffers.resize(swapChainData.imagesCount);
	constexpr uint32_t attachmentsCount = 4;
	for (size_t i = 0; i < swapChainData.imagesCount; i++) {

		VkImageView attachmentViews[attachmentsCount] = {
			gBuffer.colorTexture.getView(),
			gBuffer.normalTexture.getView(),
			gBuffer.depthTexture.getView(),
			gBuffer.positionTexture.getView()
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = attachmentsCount;
		framebufferInfo.pAttachments = attachmentViews;
		framebufferInfo.width = swapChainData.extent.width;
		framebufferInfo.height = swapChainData.extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}

		VkFramebufferCreateInfo ssaoFramebufferInfo = {};
		ssaoFramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		ssaoFramebufferInfo.renderPass = deferredResources.renderPass;
		ssaoFramebufferInfo.attachmentCount = 1;
		ssaoFramebufferInfo.pAttachments = &swapChainData.imageViews[i];
		ssaoFramebufferInfo.width = swapChainData.extent.width;
		ssaoFramebufferInfo.height = swapChainData.extent.height;
		ssaoFramebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &ssaoFramebufferInfo, nullptr, &deferredResources.frameBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create ssao framebuffer!");
		}
	}
}

void Renderer::createCommandPool()
{
	_QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

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
	gBuffer.depthTexture.cleanup(device);
	gBuffer.normalTexture.cleanup(device);
	gBuffer.colorTexture.cleanup(device);
	gBuffer.positionTexture.cleanup(device);
	vkDestroySampler(device, gBuffer.sampler, nullptr);

	for (unsigned int i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		vkDestroyFramebuffer(device, deferredResources.frameBuffers[i], nullptr);
	}

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

	for(auto &shaderPair : shaderMap.getRawMap())
	{
		ShaderData &currentShader = shaderPair.second;
		vkDestroyPipeline(device, currentShader.graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(device, currentShader.pipelineLayout, nullptr);
	}

	vkDestroyPipeline(device, deferredResources.pipeline, nullptr);
	vkDestroyPipelineLayout(device, deferredResources.pipelineLayout, nullptr);
	
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroyRenderPass(device, deferredResources.renderPass, nullptr);

	for (unsigned int i = 0; i < swapChainData.imagesCount; i++)
	{
		vkDestroyImageView(device, swapChainData.imageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChainData.swapChain, nullptr);

	for (auto &shaderPair : shaderMap.getRawMap())
		for (auto &modelPair : shaderPair.second.modelMap.getRawMap())
		{
			modelPair.second.uniformData.cleanup(device);
		}
	for (size_t i = 0; i < deferredResources.uniformBuffers.size(); i++) {
		vkDestroyBuffer(device, deferredResources.uniformBuffers[i], nullptr);
		vkFreeMemory(device, deferredResources.uniformBuffersMemory[i], nullptr);
	}

	deferredResources.uniformBuffers.clear();
	deferredResources.uniformBuffersMemory.clear();
	vkDestroyDescriptorPool(device, deferredResources.descriptorPool, nullptr);

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
	createDeferredUniformBuffers();

	for(auto &shaderPair : shaderMap.getRawMap())
	{
		populateShaderData(shaderPair.second);
	}
	createDeferredShader();
	createGBufferResources();
	createFramebuffers();
	for (auto &shaderPair : shaderMap.getRawMap())
		for (auto &modelPair : shaderPair.second.modelMap.getRawMap())
		{
			modelPair.second.uniformData.createData(device, physicalDevice, swapChainData, descriptorSetLayout, textures[modelPair.second.textureHandle.handle]);
		}
	createDeferredDescriptorSets();
	createCommandBuffers();
}

void Renderer::createDescriptorSetLayout()
{
	//geometry
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
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

	//ssao
	{

		constexpr int samplerLayoutBindingAmount = 5;

		VkDescriptorSetLayoutBinding colorSamplerLayoutBinding = {};
		colorSamplerLayoutBinding.binding = 0;

		VkDescriptorSetLayoutBinding normalSamplerLayoutBinding = {};
		normalSamplerLayoutBinding.binding = 1;

		VkDescriptorSetLayoutBinding depthSamplerLayoutBinding = {};
		depthSamplerLayoutBinding.binding = 2;

		VkDescriptorSetLayoutBinding positionSamplerLayoutBinding = {};
		positionSamplerLayoutBinding.binding = 3;
		
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 4;

		//color, normals, depth, position
		VkDescriptorSetLayoutBinding bindings[samplerLayoutBindingAmount] = { colorSamplerLayoutBinding, normalSamplerLayoutBinding, depthSamplerLayoutBinding, positionSamplerLayoutBinding, uboLayoutBinding };
		
		for(int i = 0; i < samplerLayoutBindingAmount;i++)
		{

			bindings[i].descriptorCount = 1;
			bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			bindings[i].pImmutableSamplers = nullptr;
			bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = samplerLayoutBindingAmount;
		layoutInfo.pBindings = bindings;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &deferredResources.descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create SSAO descriptor set layout!");
		}
	}

}

void Renderer::updateUniformBuffers(uint32_t currentImage)
{
	GBufferUBO gUBO;
	{
		glm::mat4 projMat = camera.getProjectionMat();
		//opengl to vulkan : y axis is inverted. Image will be rendered upside down if we don't do this
		projMat[1][1] *= -1;
		gUBO.projection = projMat;
		gUBO.view = camera.getViewMat();
		for (auto& shaderPair : shaderMap.getRawMap())
			for (auto& modelPair : shaderPair.second.modelMap.getRawMap())
			{
				Model& currentModel = modelPair.second;
				gUBO.model = currentModel.transform.getGlobalTransform();
				gUBO.color = currentModel.color;
				void* data;
				vkMapMemory(device, currentModel.uniformData.getUniformBuffersMemory()->at(currentImage), 0, sizeof(gUBO), 0, &data);
				memcpy(data, &gUBO, sizeof(gUBO));
				vkUnmapMemory(device, currentModel.uniformData.getUniformBuffersMemory()->at(currentImage));
			}
	}
	


	DeferredUBO deferredUBO;
	{
		deferredUBO.resolution = resolution;

		//set dir lights
		auto dirmap = dirLMap.getRawMap();
		int i = 0;
		for (auto& iterator : dirmap)
		{
			DirLight& light = iterator.second;
			deferredUBO.setDirLight(i, light.color, light.intensity, light.direction);
			i++;
		}

		deferredUBO.dirLightAmount = (int)dirLMap.count();

		//set point lights
		auto pointmap = pointLMap.getRawMap();
		int j = 0;
		for (auto& iterator : pointmap)
		{
			PointLight& light = iterator.second;
			deferredUBO.setPointLight(j, light.color, light.intensity, light.position);
			j++;
		}

		deferredUBO.cameraPosition = glm::vec4(camera.getTransform().getGlobalPosition(), 1.0f);
		deferredUBO.pointLightAmount = (int)pointLMap.count();

		deferredUBO.mouse = (glm::vec2)InputManager::getMousePosition();
		void* data;
		vkMapMemory(device, deferredResources.uniformBuffersMemory[currentImage],0, sizeof(DeferredUBO), 0, &data);
		memcpy(data, &deferredUBO, sizeof(DeferredUBO));
		vkUnmapMemory(device, deferredResources.uniformBuffersMemory[currentImage]);
	}
}

void Renderer::recreateCommandBufferData()
{
	vkDeviceWaitIdle(device);

	vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
	createCommandBuffers();
}

void Renderer::createDeferredShader()
{
	std::vector<char> vertShaderCode;
	readFile(std::string("shaders/deferredLighting/vert.spv"), vertShaderCode);
	std::vector<char> fragShaderCode;
	readFile(std::string("shaders/deferredLighting/frag.spv"), fragShaderCode);

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
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = 0;
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

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
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	colorBlendAttachment.blendEnable = VK_FALSE;


	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;

	VkPipelineColorBlendAttachmentState colorBlendAttachments[1] = { colorBlendAttachment };
	colorBlending.pAttachments = colorBlendAttachments;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &deferredResources.descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &deferredResources.pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_FALSE;
	depthStencil.depthWriteEnable = VK_FALSE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

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
	pipelineInfo.layout = deferredResources.pipelineLayout;
	pipelineInfo.renderPass = deferredResources.renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional


	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &deferredResources.pipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create SSAO graphics pipeline!");
	}

	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void Renderer::createDeferredUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(DeferredUBO);
	deferredResources.uniformBuffers.resize(swapChainData.imagesCount);
	deferredResources.uniformBuffersMemory.resize(swapChainData.imagesCount);

	for (size_t i = 0; i < swapChainData.imagesCount; i++) {
		bufferCreationInfo creationInfo;
		creationInfo.size = bufferSize;
		creationInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		creationInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		creationInfo.physicalDevice = physicalDevice;
		creationInfo.device = device;
		createBuffer(creationInfo, deferredResources.uniformBuffers[i], deferredResources.uniformBuffersMemory[i]);
	}
}

void Renderer::createDeferredDescriptorSets()
{
	constexpr uint32_t poolSizesCount = 5;
	VkDescriptorPoolSize poolSizes[poolSizesCount];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[4].descriptorCount = static_cast<uint32_t>(swapChainData.imagesCount);

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = poolSizesCount;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = swapChainData.imagesCount;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &deferredResources.descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create SSAO descriptor pool!");
	}
	std::vector<VkDescriptorSetLayout> layouts;
	for (unsigned int i = 0; i < swapChainData.imagesCount; i++)
	{
		layouts.push_back(deferredResources.descriptorSetLayout);
	}

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = deferredResources.descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainData.imagesCount);
	allocInfo.pSetLayouts = layouts.data();

	deferredResources.descriptorSets.resize(swapChainData.imagesCount);
	if (vkAllocateDescriptorSets(device, &allocInfo, deferredResources.descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate SSAO descriptor sets!");
	}

	for (size_t i = 0; i < swapChainData.imagesCount; i++)
	{
		VkDescriptorImageInfo colorImageInfo = {};
		colorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		colorImageInfo.imageView = gBuffer.colorTexture.getView();
		colorImageInfo.sampler = gBuffer.sampler;

		VkDescriptorImageInfo normalImageInfo = {};
		normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalImageInfo.imageView = gBuffer.normalTexture.getView();
		normalImageInfo.sampler = gBuffer.sampler;

		VkDescriptorImageInfo depthImageInfo = {};
		depthImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		depthImageInfo.imageView = gBuffer.depthTexture.getView();
		depthImageInfo.sampler = gBuffer.sampler;

		VkDescriptorImageInfo positionImageInfo = {};
		positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		positionImageInfo.imageView = gBuffer.positionTexture.getView();
		positionImageInfo.sampler = gBuffer.sampler;

		VkDescriptorBufferInfo deferredBufferInfo = {};
		deferredBufferInfo.buffer = deferredResources.uniformBuffers[i];
		deferredBufferInfo.offset = 0;
		deferredBufferInfo.range = sizeof(DeferredUBO);


		constexpr uint32_t descriptorWritesCount = 5;
		VkWriteDescriptorSet descriptorWrites[descriptorWritesCount] = {};

		//color
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = deferredResources.descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &colorImageInfo;

		//normal
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = deferredResources.descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &normalImageInfo;

		//depth
		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = deferredResources.descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &depthImageInfo;

		//position
		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = deferredResources.descriptorSets[i];
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].dstArrayElement = 0;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pImageInfo = &positionImageInfo;

		descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[4].dstSet = deferredResources.descriptorSets[i];
		descriptorWrites[4].dstBinding = 4;
		descriptorWrites[4].dstArrayElement = 0;
		descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[4].descriptorCount = 1;
		descriptorWrites[4].pBufferInfo = &deferredBufferInfo;


		vkUpdateDescriptorSets(device, descriptorWritesCount, descriptorWrites, 0, nullptr);
	}
}

void Renderer::onCreateCommandBuffers(VkCommandBuffer commandBuffer, ShaderData &shader, unsigned int i)
{
	for(auto &modelPair : shader.modelMap.getRawMap())
	{
		VertexBufferObject *currentVBO = modelPair.second.vbo;
		VkBuffer vertexBuffers[] = { currentVBO->getVertexBuffer() };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, currentVBO->getIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader.pipelineLayout, 0, 1, &modelPair.second.uniformData.getDescriptorSets()->at(i), 0, nullptr);
		
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(currentVBO->getIndices().size()), 1, 0, 0, 0);
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
	}
	throw std::runtime_error("failed to find supported format!");

	return VkFormat::VK_FORMAT_UNDEFINED;
}

VkFormat Renderer::findDepthFormat() {
	return findSupportedFormat(
		std::vector<VkFormat> { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

void Renderer::populateGBufferResource(TextureData &resource, VkFormat format, VkImageAspectFlagBits aspectFlagBits, VkImageUsageFlagBits usageFlagBits, VkImageLayout endLayout)
{
	resource.createImage(
		device,
		physicalDevice,
		swapChainData.extent.width,
		swapChainData.extent.height,
		1,
		format,
		VK_IMAGE_TILING_OPTIMAL,
		usageFlagBits,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	resource.setView(createImageView(device, resource.getImage(), format, aspectFlagBits, 1));
	TextureData::transitionImageLayout(device, commandPool, graphicsQueue, resource.getImage(), format, VK_IMAGE_LAYOUT_UNDEFINED, endLayout, 1);

}

void Renderer::createGBufferResources()
{
	//sampler
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 1.0f;
		if (vkCreateSampler(device, &samplerInfo, nullptr, &gBuffer.sampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

	populateGBufferResource(
		gBuffer.depthTexture,
		findDepthFormat(),
		VK_IMAGE_ASPECT_DEPTH_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	populateGBufferResource(
		gBuffer.normalTexture,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	populateGBufferResource(
		gBuffer.colorTexture,
		swapChainData.imageFormat,
		VK_IMAGE_ASPECT_COLOR_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	populateGBufferResource(
		gBuffer.positionTexture,
		VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_ASPECT_COLOR_BIT,
		(VkImageUsageFlagBits)(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);
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
	createDeferredUniformBuffers();
	createDescriptorSetLayout();
	createDeferredShader();
	createCommandPool();
	createGBufferResources();
	createDeferredDescriptorSets();
	createFramebuffers();
	createSyncObjects();
	createCommandBuffers();

	shaderHandle = createShader("shaders/gbuffer/frag.spv", "shaders/gbuffer/vert.spv");
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

	updateUniformBuffers(imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { currentSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

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

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapChainData.swapChain;
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
	vkDestroyDescriptorSetLayout(device, deferredResources.descriptorSetLayout, nullptr),
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
	if(ren != nullptr)
	{
		ren->resolution = glm::vec2(width, height);
		ren->framebufferResized = true;
	}
}

void Renderer::createCommandBuffers() {
	commandBuffers.resize(swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.capacity();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = renderPass;
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = swapChainData.extent;

	constexpr uint32_t clearValuesCount = 4;
	VkClearValue clearValues[clearValuesCount] = {};
	clearValues[0].color = { .0f, .0f, .0f, 1.0f };
	clearValues[2].depthStencil = { 1.0f, 0 };
	clearValues[1].color = {.0f,.0f,.0f, .0f};
	clearValues[3].color = {.0f,.0f,.0f, .0f};

	VkClearValue ssaoClearValue;
	ssaoClearValue.color = {.0f,.0f,.0f,1.0f};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValuesCount);
	renderPassInfo.pClearValues = clearValues;

	VkRenderPassBeginInfo ssaoRenderPassInfo = {};
	ssaoRenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	ssaoRenderPassInfo.renderPass = deferredResources.renderPass;
	ssaoRenderPassInfo.renderArea.offset = { 0, 0 };
	ssaoRenderPassInfo.renderArea.extent = swapChainData.extent;
	ssaoRenderPassInfo.clearValueCount = 1;
	ssaoRenderPassInfo.pClearValues = &ssaoClearValue;

	for (unsigned int i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		//geometry
		{
			renderPassInfo.framebuffer = swapChainFramebuffers[i];
			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
			for (auto& pair : shaderMap.getRawMap())
			{
				vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pair.second.graphicsPipeline);
		
				onCreateCommandBuffers(commandBuffers[i], pair.second, i);
			}
		
			vkCmdEndRenderPass(commandBuffers[i]);
		}
		

		//ssao
		{
			ssaoRenderPassInfo.framebuffer = deferredResources.frameBuffers[i];
			vkCmdBeginRenderPass(commandBuffers[i], &ssaoRenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, deferredResources.pipeline);
			VkDeviceSize offsets[] = { 0 };

			//uniforms
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, deferredResources.pipelineLayout, 0, 1, &deferredResources.descriptorSets[i], 0, nullptr);
			vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);
		}
		

		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}
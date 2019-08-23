#pragma once
#ifndef VBO_DEFINED
#define VBO_DEFINED

#include "Vulkan/vulkan.h"
#include <vector>
#include "Vertex.h"
#include "Utilities.h"

struct VBOCreationInfo
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
};

class VertexBufferObject
{
public:

	VertexBufferObject(VBOCreationInfo &creationInfo, const char * path);
	VertexBufferObject(){};
	~VertexBufferObject(){ vertices.clear(); indices.clear(); vertexBuffer = nullptr; indexBuffer = nullptr; vertexBufferMemory = nullptr; indexBufferMemory = nullptr; };

	void cleanup(VkDevice device);

	void createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	void createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);

	void loadModel(const char * path);

	VkBuffer getVertexBuffer();

	VkBuffer getIndexBuffer();

	std::vector<Vertex> getVertices();

	std::vector<uint32_t> getIndices();

	void setVertices(std::vector<Vertex> givenVertices);
	void setIndices(std::vector<uint32_t> givenIndices);

	box getBox();

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer = nullptr;
	VkDeviceMemory vertexBufferMemory = nullptr;
	VkBuffer indexBuffer = nullptr;
	VkDeviceMemory indexBufferMemory = nullptr;
};

#endif
#pragma once
#ifndef VBO_DEFINED
#define VBO_DEFINED

#include "Vulkan/vulkan.h"
#include <vector>
#include "Vertex.h"
#include "Utilities.h"
#include <string>


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


	static VertexBufferObject create(std::string name);

	static void destroy(VertexBufferObject &givenVBO);

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



	static void getUnitCube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

private:
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	VkBuffer vertexBuffer = nullptr;
	VkDeviceMemory vertexBufferMemory = nullptr;
	VkBuffer indexBuffer = nullptr;
	VkDeviceMemory indexBufferMemory = nullptr;
};

#endif
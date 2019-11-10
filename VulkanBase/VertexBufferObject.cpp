#include "VertexBufferObject.h"
#include "VulkanUtilities.h"
#include <fstream>
#include "Renderer.h"
#include "Singleton.h"

typedef char byte;

VertexBufferObject::VertexBufferObject(VBOCreationInfo & creationInfo, const char * path)
{
	loadModel(path);
	createVertexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
	createIndexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
}

void VertexBufferObject::createIndexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();

	bufferCreationInfo stagingBufferCreationInfo;
	stagingBufferCreationInfo.size = bufferSize;
	stagingBufferCreationInfo.device = device;
	stagingBufferCreationInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreationInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreationInfo.physicalDevice = physicalDevice;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(stagingBufferCreationInfo, stagingBuffer, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	bufferCreationInfo indexBufferCreationInfo;
	indexBufferCreationInfo.size = bufferSize;
	indexBufferCreationInfo.device = device;
	indexBufferCreationInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	indexBufferCreationInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	indexBufferCreationInfo.physicalDevice = physicalDevice;

	createBuffer(indexBufferCreationInfo, indexBuffer, indexBufferMemory);

	copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VertexBufferObject::createVertexBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue)
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();
	bufferCreationInfo stagingBufferCreationInfo;
	stagingBufferCreationInfo.size = bufferSize;
	stagingBufferCreationInfo.device = device;
	stagingBufferCreationInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	stagingBufferCreationInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	stagingBufferCreationInfo.physicalDevice = physicalDevice;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(
		stagingBufferCreationInfo,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	bufferCreationInfo vertexBufferCreationInfo;
	vertexBufferCreationInfo.size = bufferSize;
	vertexBufferCreationInfo.device = device;
	vertexBufferCreationInfo.properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	vertexBufferCreationInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertexBufferCreationInfo.physicalDevice = physicalDevice;

	createBuffer(
		vertexBufferCreationInfo,
		vertexBuffer,
		vertexBufferMemory);

	copyBuffer(device, commandPool, graphicsQueue, stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void VertexBufferObject::loadModel(const char * path)
{
	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (file.fail()) throw std::runtime_error("File open failed!");
	size_t fileByteCount = file.tellg();
	byte *data = new byte[fileByteCount];
	file.clear();
	file.seekg(0, std::ios::beg);
	file.read(data, fileByteCount);
	file.close();
	
	size_t vertexByteCount = 0;
	std::copy(data, &data[sizeof(size_t)], (char*)&vertexByteCount);
	
	vertices.resize(vertexByteCount / sizeof(Vertex));
	std::copy((&data[sizeof(size_t)]), (&data[vertexByteCount + sizeof(size_t)]), (char*)vertices.data());

	size_t indexByteCount = 0;
	std::copy(&data[sizeof(size_t) + vertexByteCount], &data[sizeof(size_t)*2 + vertexByteCount], (char*)&indexByteCount);
	indices.resize(indexByteCount / sizeof(uint32_t));
	std::copy((&data[sizeof(size_t)*2 + vertexByteCount]), (&data[sizeof(size_t) * 2 + vertexByteCount + indexByteCount]), (char*)indices.data());
	
	delete[] data;
}

VkBuffer VertexBufferObject::getVertexBuffer()
{
	return vertexBuffer;
}

VkBuffer VertexBufferObject::getIndexBuffer()
{
	return indexBuffer;
}

std::vector<Vertex> VertexBufferObject::getVertices()
{
	return vertices;
}

std::vector<uint32_t> VertexBufferObject::getIndices()
{
	return indices;
}

void VertexBufferObject::setVertices(std::vector<Vertex> givenVertices)
{
	vertices = givenVertices;
}

void VertexBufferObject::setIndices(std::vector<uint32_t> givenIndices)
{
	indices = givenIndices;
}

box VertexBufferObject::getBox()
{
	box returnBox;

	float smallestX = .0f, smallestY = .0f, smallestZ = .0f;
	float largestX = .0f, largestY = .0f, largestZ = .0f;

	for(Vertex v : vertices)
	{
		if (v.pos.x > largestX) largestX = v.pos.x;
		if (v.pos.y > largestY) largestY = v.pos.y;
		if (v.pos.z > largestZ) largestZ = v.pos.z;
		if (v.pos.x < smallestX) largestX = v.pos.x;
		if (v.pos.y < smallestY) largestY = v.pos.y;
		if (v.pos.z < smallestZ) largestZ = v.pos.z;
	}


	returnBox.bottomLeftNear = { smallestX,smallestY,smallestZ };
	returnBox.topRightFar = { largestX,largestY,largestZ };

	return returnBox;
}

void VertexBufferObject::getUnitCube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
	glm::vec3 positions[8] =
	{
		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3(-1.0f,-1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f,-1.0f),
		glm::vec3(-1.0f, 1.0f,-1.0f),
		glm::vec3(1.0f,-1.0f, 1.0f),
		glm::vec3(1.0f,-1.0f,-1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
	};

	uint32_t ind[36] =
	{
		0,	1,	2,
		0,	2,	4,
		5,  7,	1,
		7,	2,	1,
		1,	6,	5,
		1,	0,	6,
		4,	3,	6,
		0,	4,	6,
		7,	3,	2,
		3,	4,	2,
		6,	3,	5,
		3,	7,	5
	};

	vertices.resize(8);
	indices.resize(36);


	for (unsigned int i = 0; i < 8; i++)
	{
		vertices[i].pos = positions[i];
		vertices[i].texCoord = glm::vec2(.0f, .0f);
	}

	for (unsigned int i = 0; i < 36; i++)
	{
		indices[i] = ind[i];
	}
}

VertexBufferObject VertexBufferObject::create(std::string name)
{
	auto creationInfo = Singleton<Renderer>::getInstance()->getVBOCreationInfo();
	return VertexBufferObject(creationInfo, name.c_str());
}

void VertexBufferObject::destroy(VertexBufferObject & givenVBO)
{
	VkDevice device = Singleton<Renderer>::getInstance()->getVBOCreationInfo().device;

	vkDestroyBuffer(device, givenVBO.indexBuffer, nullptr);
	vkFreeMemory(device, givenVBO.indexBufferMemory, nullptr);

	vkDestroyBuffer(device, givenVBO.vertexBuffer, nullptr);
	vkFreeMemory(device, givenVBO.vertexBufferMemory, nullptr);

	givenVBO.indexBuffer = nullptr;
	givenVBO.indexBufferMemory = nullptr;
	givenVBO.vertexBuffer = nullptr;
	givenVBO.vertexBufferMemory = nullptr;
	givenVBO.vertices.clear();
	givenVBO.indices.clear();
}

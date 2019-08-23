#pragma once
#ifndef VERTEX_DEFINED
#define VERTEX_DEFINED

#include "glm.hpp"
#include "Vulkan/vulkan.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec2 texCoord;

	static VkVertexInputBindingDescription getBindingDescription();

	static VkVertexInputAttributeDescription *getAttributeDescriptions(unsigned int &outSize);

	bool operator==(const Vertex& other) const {
		return pos == other.pos && texCoord == other.texCoord;
	}
};

#endif // !VERTEX_DEFINED

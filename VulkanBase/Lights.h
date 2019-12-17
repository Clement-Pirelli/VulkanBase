#pragma once
#include "glm.hpp"

struct DirectionalLight
{
	glm::vec4 direction;
	glm::vec4 color;
	float intensity;
};

struct PointLight
{
	glm::vec4 position;
	glm::vec4 color;
	float intensity;
};
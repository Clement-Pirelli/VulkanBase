#pragma once
#ifndef MODEL_DEFINED
#define MODEL_DEFINED

#include "Transform.h"
#include "Renderer.h"
#include "UniformData.h"

struct Model
{
	TextureHandle textureHandle;
	VBOHandle vboHandle;
	UniformData uniformData;
	Transform *transform = nullptr;
	glm::vec4 color;
};

#endif
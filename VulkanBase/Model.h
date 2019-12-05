#pragma once
#ifndef MODEL_DEFINED
#define MODEL_DEFINED

#include "Transform.h"
#include "Renderer.h"
#include "UniformData.h"

struct Model
{
	Model(VertexBufferObject *givenVBO) : vbo(givenVBO){}
	Model(){}
	TextureHandle textureHandle = {};
	VertexBufferObject *vbo = nullptr;
	UniformData uniformData = {};
	Transform transform = Transform();
	glm::vec4 color = glm::vec4();
};

#endif
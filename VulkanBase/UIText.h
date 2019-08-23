#pragma once
#include <vector>
#include "Vertex.h"
#include "glm.hpp"

class Renderer;
class VertexBufferObject;
class Model;
class Transform;

class UIText
{
public:
	UIText(const char *fontTexturePath, const char *text, Transform *givenTransform, glm::vec3 givenColor);
	~UIText();

private:

	void setText(const char *givenText, glm::vec3 givenColor);
	VertexBufferObject *vbo;
	Model *model;
	Transform *transform;
};
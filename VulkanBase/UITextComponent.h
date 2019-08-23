#pragma once
#include "Component.h"
#include "glm.hpp"

class VertexBufferObject;
class Model;

class UITextComponent : public Component
{
public:
	UITextComponent(const char *givenFontTexturePath, const char *givenText, glm::vec4 givenColor);
	~UITextComponent();

	void onInit() override;
	void onActive() override;
	void onInactive() override;

	void setColor(glm::vec4 givenColor);
	void setText(const char *text);

private:
	void resetVBO();
	void setVBO();
	VertexBufferObject *vbo;
	Model *model;
	const char *fontTexturePath, *text;
	glm::vec4 color;
};


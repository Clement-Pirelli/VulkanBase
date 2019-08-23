#pragma once
#include "Component.h"
#include "glm.hpp"

class Model;

class MeshComponent : public Component
{
public:
	MeshComponent(const char *givenTexturePath, const char *givenVboPath);
	~MeshComponent();

	void onInit() override;
	void onInactive() override;
	void onActive() override;

	void setColor(glm::vec4 givenColor);

private:
	Model *model;
	const char *texturePath, *vboPath;
};


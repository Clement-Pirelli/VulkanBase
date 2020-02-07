#pragma once
#include "Transform.h"

class BoidController;

class Boid
{
public:
	Boid() : controller(nullptr) {}

	void init(BoidController *givenController, glm::vec3 givenDirection, size_t givenIndex);
	void update(float deltaTime);
	void lateUpdate(float deltaTime);

private:

	BoidController *controller;
	size_t myIndex = 0;
	glm::vec3 direction = glm::vec3(1.0f,.0f,.0f);
};


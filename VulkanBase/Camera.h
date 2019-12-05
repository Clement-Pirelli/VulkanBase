#pragma once
#include "glm.hpp"
#include "Transform.h"

class Camera
{
public:
	Camera(const Transform &givenTransform, float givenAspectRatio, float givenDegFOV, float givenZNear, float givenZFar);
	~Camera();
	
	float getDegFOV() const;
	float getZNear() const;
	float getZFar() const;

	glm::mat4 getProjectionMat() const;
	glm::mat4 getViewMat() const;
	Transform &getTransform();

	glm::vec3 getBackward() const;
	glm::vec3 getUp() const;
	glm::vec3 getRight() const;

private:
	float degFOV = 60.0f, zNear = 1.0f, zFar = 200.0f, aspectRatio = 9.0f/6.0f;
	Transform transform = Transform();
};


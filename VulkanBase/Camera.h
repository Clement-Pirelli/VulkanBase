#pragma once
#include "glm.hpp"

class Transform;
class Camera
{
public:
	Camera(Transform *givenTransform, float givenAspectRatio, float givenDegFOV, float givenZNear, float givenZFar);
	~Camera();
	
	float getDegFOV();
	float getZNear();
	float getZFar();

	glm::mat4 getProjectionMat();
	glm::mat4 getViewMat();

private:
	float degFOV = .0f, zNear = .0f, zFar = .0f, aspectRatio = 1.0f;
	Transform *transform = nullptr;
};


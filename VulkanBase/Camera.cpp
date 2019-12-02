#include "Camera.h"
#include "Singleton.h"
#include "Renderer.h"
#include "Transform.h"

Camera::Camera(Transform *givenTransform, float givenAspectRatio, float givenDegFOV, float givenZNear, float givenZFar) : transform(givenTransform), aspectRatio(givenAspectRatio), degFOV(givenDegFOV), zNear(givenZNear), zFar(givenZFar){}

Camera::~Camera(){}

float Camera::getDegFOV()
{
	return degFOV;
}

float Camera::getZNear()
{
	return zNear;
}

float Camera::getZFar()
{
	return zFar;
}

glm::mat4 Camera::getProjectionMat()
{
	return glm::perspective(glm::radians(degFOV), aspectRatio, zNear, zFar);
}

glm::mat4 Camera::getViewMat()
{
	glm::mat4 tra = glm::mat4(1.0f);
	tra = glm::translate(tra, -transform->getGlobalPosition());
	glm::mat4 rot = glm::mat4(1.0f);
	glm::vec3 rotVec = transform->getGlobalRotation();
	rot = glm::rotate(rot, rotVec.x, glm::vec3(1.0f,.0f,.0f));
	rot = glm::rotate(rot, rotVec.y, glm::vec3(.0f, 1.0f, .0f));
	rot = glm::rotate(rot, rotVec.z, glm::vec3(.0f, .0f, 1.0f));

	return tra * rot;
}
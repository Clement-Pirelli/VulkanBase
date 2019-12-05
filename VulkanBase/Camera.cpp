#include "Camera.h"
#include "Singleton.h"
#include "Renderer.h"
#include "Transform.h"

Camera::Camera(const Transform &givenTransform, float givenAspectRatio, float givenDegFOV, float givenZNear, float givenZFar) : transform(givenTransform), aspectRatio(givenAspectRatio), degFOV(givenDegFOV), zNear(givenZNear), zFar(givenZFar){}

Camera::~Camera(){}

float Camera::getDegFOV()const
{
	return degFOV;
}

float Camera::getZNear() const
{
	return zNear;
}

float Camera::getZFar() const
{
	return zFar;
}

glm::vec3 Camera::getBackward() const
{
	const glm::mat4 viewMat = getViewMat();
	return glm::vec3(viewMat[0][2], viewMat[1][2], viewMat[2][2]);;
}

glm::vec3 Camera::getUp() const
{
	const glm::mat4 viewMat = getViewMat(); 
	return glm::vec3(viewMat[0][1], viewMat[1][1], viewMat[2][1]);
}

glm::vec3 Camera::getRight() const
{
	const glm::mat4 viewMat = getViewMat();
	return glm::vec3(viewMat[0][0], viewMat[1][0], viewMat[2][0]);
}

glm::mat4 Camera::getProjectionMat()const
{
	return glm::perspective(glm::radians(degFOV), aspectRatio, zNear, zFar);
}

glm::mat4 Camera::getViewMat()const
{
	glm::mat4 transformMat = transform.getGlobalPositionMat() * transform.getGlobalRotationMat();
	return glm::inverse(transformMat);
}

Transform &Camera::getTransform()
{
	return transform;
}

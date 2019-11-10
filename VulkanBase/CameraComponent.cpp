#include "CameraComponent.h"
#include "Singleton.h"
#include "Renderer.h"

#include "Entity.h"
#include "Transform.h"

CameraComponent::CameraComponent(float givenDegFOV, float givenZNear, float givenZFar) : degFOV(givenDegFOV), zNear(givenZNear), zFar(givenZFar){}

CameraComponent::~CameraComponent()
{
	Singleton<Renderer>::getInstance()->setCamera(nullptr);
}

float CameraComponent::getDegFOV()
{
	return degFOV;
}

float CameraComponent::getZNear()
{
	return zNear;
}

float CameraComponent::getZFar()
{
	return zFar;
}

glm::mat4 CameraComponent::getProjectionMat()
{
	return glm::perspective(glm::radians(degFOV), 1920.0f / 1080.0f, zNear, zFar);
}

glm::mat4 CameraComponent::getViewMat()
{
	Transform *transform = owner->getTransform();
	glm::mat4 tra = glm::mat4(1.0f);
	tra = glm::translate(tra, -transform->getGlobalPosition());
	glm::mat4 rot = glm::mat4(1.0f);
	glm::vec3 rotVec = transform->getGlobalRotation();
	rot = glm::rotate(rot, rotVec.x, glm::vec3(1.0f,.0f,.0f));
	rot = glm::rotate(rot, rotVec.y, glm::vec3(.0f, 1.0f, .0f));
	rot = glm::rotate(rot, rotVec.z, glm::vec3(.0f, .0f, 1.0f));



	return tra * rot;
}

void CameraComponent::onActive()
{
	Singleton<Renderer>::getInstance()->setCamera(this);
}

void CameraComponent::onInactive()
{
	Singleton<Renderer>::getInstance()->setCamera(nullptr);
}

void CameraComponent::onInit()
{
	Singleton<Renderer>::getInstance()->setCamera(this);
}

void CameraComponent::onUpdate(float deltaTime)
{
	if(shakeTime > .0f)
	{
		shakeTime -= deltaTime;
		//TODO : SCREEN SHAKE USING SHAKE INTENSITY HERE
	}
}

void CameraComponent::screenShake(float time, float intensity)
{
	shakeTime = time;
	shakeIntensity = intensity;
}

#include "CameraComponent.h"
#include "Singleton.h"
#include "Renderer.h"


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

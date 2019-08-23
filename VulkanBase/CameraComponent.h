#pragma once
#include "Component.h"
class CameraComponent : public Component
{
public:
	CameraComponent(float givenDegFOV, float givenZNear, float givenZFar);
	~CameraComponent();
	
	float getDegFOV();
	float getZNear();
	float getZFar();

	void onActive() override;
	void onInactive() override;
	void onInit() override;
	void onUpdate(float deltaTime) override;

	void screenShake(float time, float intensity);

private:
	float degFOV = .0f, zNear = .0f, zFar = .0f;

	float shakeTime = .0f;
	float shakeIntensity = .0f;
};


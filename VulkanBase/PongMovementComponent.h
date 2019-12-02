#pragma once
#include "Component.h"
#include "glm.hpp"

class PlayerInputComponent;

class PongMovementComponent : public Component
{
public:
	PongMovementComponent(PlayerInputComponent *givenInputComponent, float givenSpeed = 1.0f);
	~PongMovementComponent();
	void onInit() override;
	void onUpdate(float deltaTime) override;
	void onLateUpdate(float deltaTime) override;
	void onCollision(Entity *other) override;
private:

	glm::vec3 direction = glm::vec3(.0f);
	glm::vec3 lastPosition = glm::vec3(.0f);
	float speed = .0f;
	PlayerInputComponent *inputComponent;
};


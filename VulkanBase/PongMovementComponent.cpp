#include "PongMovementComponent.h"
#include "PlayerInputComponent.h"
#include "Transform.h"
#include "Entity.h"


PongMovementComponent::PongMovementComponent(float givenSpeed)
{
	speed = givenSpeed;
	direction.x = direction.y = direction.z = .0f;
}


PongMovementComponent::~PongMovementComponent()
{
	inputComponent = nullptr;
}

void PongMovementComponent::onInit()
{
	if(!owner->requestComponent(inputComponent))
	{
		throw std::runtime_error("PongMovementComponent : owner didn't have an input component!");
	}
	lastPosition = owner->getTransform()->getLocalPosition();
}

void PongMovementComponent::onUpdate(float deltaTime)
{
	glm::vec2 inputDir = inputComponent->getInputDirection();
	
	direction = -glm::vec3(inputDir.x, inputDir.y, .0f);

	owner->getTransform()->addLocalPosition(direction*speed*deltaTime);
}

void PongMovementComponent::onLateUpdate(float deltaTime)
{
	lastPosition = owner->getTransform()->getLocalPosition();
}

void PongMovementComponent::onCollision(Entity * other)
{
	owner->getTransform()->setLocalPosition(lastPosition);
}
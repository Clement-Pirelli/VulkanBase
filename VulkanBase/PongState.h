#pragma once
#include "State.h"
#include "Transform.h"
#include "Renderer.h"

class EntityManager;

class PongState :
	public State
{
public:
	PongState(StateMachine *givenStateMachine, EntityManager *givenEntityManager);
	~PongState();

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;

private:
	EntityManager *entityManager = nullptr;

	unsigned int playerScore = 0, botScore = 0;
	
	float rotationAccumulator = .0f;
	ModelHandle modelHandle;
	Transform modelTransform;
};


#pragma once
#include "State.h"

class UITextComponent;
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

	UITextComponent *textComponent = nullptr;
	EntityManager *entityManager = nullptr;

	unsigned int playerScore = 0, botScore = 0;
};


#include "StateMachine.h"
#include "State.h"

void StateMachine::onUpdate(float deltaTime)
{
	if (currentState != nullptr)
		currentState->onUpdate(deltaTime);
}

void StateMachine::nextState(State *givenState)
{
	if (lastState != nullptr)
	{
		delete lastState;
		lastState = nullptr;
	}
	
	currentState->onExit();
	lastState = currentState;
	currentState = givenState;
	currentState->onEnter();
}

void StateMachine::setFirstState(State *givenState)
{
	if (currentState == nullptr)
	{
		currentState = givenState;
		currentState->onEnter();
	} 
}


#pragma once

class StateMachine;

class State
{
public:
	State(StateMachine *givenStateMachine) : stateMachine(givenStateMachine) {}
	virtual ~State(){};
	virtual void onEnter() = 0;
	virtual void onExit() = 0;
	virtual void onUpdate(float deltaTime) = 0;

	StateMachine *stateMachine;
};
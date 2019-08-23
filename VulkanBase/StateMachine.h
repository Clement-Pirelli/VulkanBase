#pragma once

class State;

class StateMachine
{
public:
	StateMachine(State *firstState = nullptr){ currentState = firstState; };
	~StateMachine(){};

	void onUpdate(float deltaTime);
	void nextState(State *givenState);
	void setFirstState(State *givenState);

protected :

	State *currentState = nullptr;
	State *lastState = nullptr;
};
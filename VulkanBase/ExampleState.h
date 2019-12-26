#pragma once
#include "State.h"
#include "Renderer.h"
#include "Input.h"

class ExampleState :
	public State
{
public:
	ExampleState(StateMachine *givenStateMachine);
	~ExampleState();

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;

private:
	std::vector<ModelHandle> modelHandles = {};
	Transform *cameraTransform = nullptr;
	Input *input = nullptr;
	Renderer *renderer = nullptr;
	float movementSpeed = 3.0f;
	std::vector<PointLightHandle> pLightHandles;
};


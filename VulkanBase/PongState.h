#pragma once
#include "State.h"
#include "Renderer.h"
#include "Input.h"

class PongState :
	public State
{
public:
	PongState(StateMachine *givenStateMachine);
	~PongState();

	void onEnter() override;
	void onExit() override;
	void onUpdate(float deltaTime) override;

private:
	ShaderHandle shaderHandle = {};
	std::vector<ModelHandle> modelHandles = {};
	Transform *cameraTransform = nullptr;
	Input *input = nullptr;
	Renderer *renderer = nullptr;
	float movementSpeed = 3.0f;
};


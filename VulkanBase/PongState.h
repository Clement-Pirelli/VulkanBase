#pragma once
#include "State.h"
#include "Renderer.h"

class EntityManager;
class Camera;

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

	ShaderHandle shaderHandle;
	std::vector<ModelHandle> modelHandles;
	std::vector<Transform*> transforms;
	Camera *camera = nullptr;
	Transform *cameraTransform = nullptr;
};


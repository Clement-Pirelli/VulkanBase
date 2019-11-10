#include "PongState.h"
#include "EntityManager.h"
#include "Entity.h"
#include <string>

#include "glfw3.h"
#include "PlayerInputComponent.h"
#include "PongMovementComponent.h"
#include "CameraComponent.h"
#include "VertexBufferObject.h"
#include "Vertex.h"
#include "Singleton.h"
#include "Renderer.h"


PongState::PongState(StateMachine *givenStateMachine, EntityManager *givenEntityManager) : State(givenStateMachine), entityManager(givenEntityManager)
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	modelTransform.setLocalPosition(glm::vec3(.0f,.0f,.0f));
	modelTransform.setLocalScale(glm::vec3(.01f,.01f,.01f));
	modelHandle = renderer->createModel(&modelTransform, "_assets/textures/chalet.jpg", "_assets/meshes/fox.o", glm::vec4(1.0f,1.0f,1.0f,1.0f));
}


PongState::~PongState()
{
}

void PongState::onEnter()
{

}

void PongState::onExit()
{
	entityManager->clearEntities();
}

void PongState::onUpdate(float deltaTime)
{
	rotationAccumulator += deltaTime;
	modelTransform.setLocalRotation(glm::vec3(rotationAccumulator, rotationAccumulator, .0f));

	if(rotationAccumulator > 10.0f)
	{
		Renderer *renderer = Singleton<Renderer>::getInstance();
		renderer->removeModel(modelHandle);
	}

	entityManager->onUpdate(deltaTime);
	entityManager->onLateUpdate(deltaTime);
}

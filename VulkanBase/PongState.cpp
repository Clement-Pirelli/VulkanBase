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
	modelTransform.setLocalScale(glm::vec3(1.0f,1.0f,1.0f));
	shaderHandle = renderer->createShader("shaders/normalfrag.spv","shaders/normalvert.spv");
	modelHandle = renderer->createModel(shaderHandle, &modelTransform, "_assets/textures/chalet.jpg", "_assets/meshes/chalet.o", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	
	modelTransform2.setLocalPosition(glm::vec3(1.0f, 1.0f, .0f));
	modelTransform2.setLocalScale(glm::vec3(1.0f, 1.0f, 2.0f));
	shaderHandle2 = renderer->createShader("shaders/changedfrag.spv", "shaders/changedvert.spv");
	modelHandle2 = renderer->createModel(shaderHandle2, &modelTransform2, "_assets/textures/chalet.jpg", "_assets/meshes/chalet.o", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));



	entityManager->addEntity(new Entity(new CameraComponent(90.0f, .01f,200.0f), new Transform(glm::vec3(1.0f,1.0f,1.0f), glm::vec3(.0f, .0f,5.0f), glm::vec3(.0f,.0f,.0f))));
}


PongState::~PongState()
{
	entityManager->clearEntities();
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
		renderer->removeModel(modelHandle, shaderHandle);
	}

	entityManager->onUpdate(deltaTime);
	entityManager->onLateUpdate(deltaTime);
}

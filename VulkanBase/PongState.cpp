#include "PongState.h"
#include "UITextComponent.h"
#include "EntityManager.h"
#include "Entity.h"
#include "Transform.h"
#include <string>

#include "glfw3.h"
#include "MeshComponent.h"
#include "UITextComponent.h"
#include "PlayerInputComponent.h"
#include "PongMovementComponent.h"
#include "CameraComponent.h"
#include "PongPaddleMeshComponent.h"


PongState::PongState(StateMachine *givenStateMachine, EntityManager *givenEntityManager) : State(givenStateMachine), entityManager(givenEntityManager)
{}


PongState::~PongState()
{
}

void PongState::onEnter()
{
	Transform *paddleTransform = new Transform(glm::vec3(1.0f), glm::vec3(.0f, .0f, -10.0f), glm::vec3(.0f));
	std::vector<Component *> paddleComponents;
	CameraComponent *cameraComp = new CameraComponent(75.0f, .1f, 100.0f);
	paddleComponents.push_back(cameraComp);
	//paddleComponents.push_back(new PongPaddleMeshComponent(cameraComp));
	paddleComponents.push_back(new MeshComponent("_assets/textures/monkigoose.png", "_assets/meshes/ship.o"));
	paddleComponents.push_back(new PlayerInputComponent(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D));
	PlayerInputComponent::setMouseSensitivity(.001f);
	paddleComponents.push_back(new PongMovementComponent(10.0f));
	Entity *cameraEntity = new Entity(paddleComponents, paddleTransform);
	entityManager->addEntity(cameraEntity);

	//Transform *shipTransform = new Transform(paddleTransform, glm::vec3(1.0f), glm::vec3(.0f, .0f, 10.0f), glm::vec3(.0f));
	//std::vector<Component *> shipComponents;
	////shipComponents.push_back(new MeshComponent("_assets/textures/monkigoose.png", "_assets/meshes/ship.o"));
	//shipComponents.push_back(new PongPaddleMeshComponent(cameraComp));
	//
	//Entity *shipEntity = new Entity(shipComponents, shipTransform);
	//entityManager->addEntity(shipEntity);

	Transform *foxTransform = new Transform(glm::vec3(.01f), glm::vec3(1.0f, .0f, .0f), glm::vec3(.0f));
	MeshComponent *foxMesh = new MeshComponent("_assets/textures/foxtex.png", "_assets/meshes/fox.o");
	Entity *foxEntity = new Entity(foxMesh, foxTransform);
	entityManager->addEntity(foxEntity);

	Transform *textTransform = new Transform(glm::vec3(.05f), glm::vec3(-.9f, .9f, .0f), glm::vec3(.0f));
	textComponent = new UITextComponent("_assets/textures/font2.png", "lorem ipsum dolor sit amet", glm::vec4(1.0f));
	Entity *UITextEntity = new Entity(textComponent, textTransform);
	entityManager->addEntity(UITextEntity);
}

void PongState::onExit()
{
	entityManager->clearEntities();
}

void PongState::onUpdate(float deltaTime)
{
	textComponent->setText(std::to_string((int)(1.0f/deltaTime)).c_str());

	entityManager->onUpdate(deltaTime);
	entityManager->onLateUpdate(deltaTime);
}

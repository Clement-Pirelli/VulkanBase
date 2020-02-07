#include "ExampleState.h"
#include <string>
#include "glfw3.h"
#include "Transform.h"
#include "Camera.h"
#include "Singleton.h"
#include "Renderer.h"
#include "Utilities.h"


ExampleState::ExampleState(StateMachine *givenStateMachine) : State(givenStateMachine)
{
	renderer = Singleton<Renderer>::getInstance();
	//set the camera
	Transform camTrans = Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(.0f, .0f, 5.0f), glm::vec3(.0f, .0f, .0f));
	Camera &camera = renderer->getCamera();
	camera = Camera(camTrans, 9.0f / 6.0f, 60.0f, .01f, 200.0f);
	cameraTransform = &camera.getTransform();

	input = new Input(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_C, GLFW_KEY_X);

	dirLightHandle = renderer->createDirLight(glm::vec3(.0f,-1.0f,.0f), glm::vec3(1.0f,.8f,.6f), 1.0f);
	assert(isHandleValid(dirLightHandle));
}


ExampleState::~ExampleState()
{
	delete input;
}

void ExampleState::onEnter()
{

}

void ExampleState::onExit()
{
}

void ExampleState::onUpdate(float deltaTime)
{
	input->onUpdate();

	//mouse movement
	glm::vec2 mouseDelta = -input->getMouseDelta();
	cameraTransform->addLocalRotation(glm::vec3(.0f, mouseDelta.x, .0f)*deltaTime);
	cameraTransform->addLocalRotation(glm::vec3(mouseDelta.y, .0f, .0f)*deltaTime);


	//keyboard movement
	glm::vec3 inputDir = input->getInputDirection();
	
	Camera &camera = renderer->getCamera();
	glm::vec3 cameraRight = camera.getRight(), 
		cameraBackward = camera.getBackward(), 
		cameraUp = camera.getUp();
	
	bool goingFast = input->isGoingFast();

	glm::vec3 cameraInputDir = inputDir.z * cameraBackward + inputDir.y * cameraUp + inputDir.x * cameraRight;
	cameraTransform->addLocalPosition(cameraInputDir*deltaTime*movementSpeed * (goingFast ? 10.0f : 1.0f));
	
	controller.update(deltaTime);

	input->onLateUpdate();
}

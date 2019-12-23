#include "ExampleState.h"
#include <string>
#include "glfw3.h"
#include "Transform.h"
#include "Camera.h"
#include "Singleton.h"
#include "Renderer.h"
#include "Utilities.h"
#include "Model.h"


ExampleState::ExampleState(StateMachine *givenStateMachine) : State(givenStateMachine)
{
	renderer = Singleton<Renderer>::getInstance();
	shaderHandle = renderer->createShader("shaders/frag.spv", "shaders/vert.spv");

	//models

	modelHandles.resize(2);
	Transform lionTransform = Transform(glm::vec3(.01f, .01f, .01f), glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, .0f, .0f));
	modelHandles[0] = renderer->createModel(shaderHandle, lionTransform, "_assets/textures/white.png", "_assets/meshes/lion.o", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	Transform planeTransform = Transform(glm::vec3(10.0f, 10.0f, 10.0f), glm::vec3(.0f, .0f, .0f), glm::vec3(.0f, .0f, .0f));
	modelHandles[1] = renderer->createModel(shaderHandle, planeTransform, "_assets/textures/white.png", "_assets/meshes/plane.o", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

	//set the camera
	Transform camTrans = Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(.0f, 1.0f, 5.0f), glm::vec3(.0f, .0f, .0f));
	Camera& camera = renderer->getCamera();
	camera = Camera(camTrans, 9.0f / 6.0f, 45.0f, .01f, 200.0f);
	cameraTransform = &camera.getTransform();

	input = new Input(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_C, GLFW_KEY_X);
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

	glm::vec3 cameraInputDir = inputDir.z * cameraBackward + inputDir.y * cameraUp + inputDir.x * cameraRight;
	cameraTransform->addLocalPosition(cameraInputDir*deltaTime*movementSpeed);
	
	input->onLateUpdate();
}

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
	constexpr int modelCount = 1000;
	renderer = Singleton<Renderer>::getInstance();
	shaderHandle = renderer->createShader("shaders/frag.spv", "shaders/vert.spv");
	std::vector<Transform> transforms(modelCount);
	modelHandles.resize(modelCount);
	std::vector<glm::vec4> colors;
	colors.resize(modelCount);

	//random placement of models
	for(int i = 0; i < modelCount; i++)
	{
		float scaleScalar = (float)(Util::rand() % 5)*.025f;
		glm::vec3 scale(scaleScalar, scaleScalar, scaleScalar);
		glm::vec3 position((float)(Util::rand() % 100), (float)(Util::rand() % 100), (float)(Util::rand() % 100));
		position *= .16f;
		glm::vec3 rotation((float)(Util::rand() % 360), (float)(Util::rand() % 360), (float)(Util::rand() % 360));
		transforms[i] = new Transform(scale, position, rotation);
		colors[i] = glm::vec4((Util::rand() % 100) / 100.0f, (Util::rand() % 100) / 100.0f, (Util::rand() % 100) / 100.0f, 1.0f)*.5f + glm::vec4(1.0f,1.0f,1.0f,1.0f)*.5f;
	}

	modelHandles = renderer->createModels(shaderHandle, transforms, "_assets/textures/robot.jpg", "_assets/meshes/robot.o", colors);

	//set the camera
	Transform camTrans = Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(.0f, .0f, 5.0f), glm::vec3(.0f, .0f, .0f));
	Camera &camera = renderer->getCamera();
	camera = Camera(camTrans, 9.0f / 6.0f, 60.0f, .01f, 200.0f);
	cameraTransform = &camera.getTransform();

	input = new Input(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_C, GLFW_KEY_X);

	dirLightHandle = renderer->createDirLight(glm::vec3(.0f,-1.0f,.0f), glm::vec3(1.0f,.8f,.6f), 1.0f);
	assert(isHandleValid(dirLightHandle));

	pointLightHandle = renderer->createPointLight(glm::vec3(.0f,.0f,.0f), glm::vec3(.1f,.2f,.1f), 1.0f);

	renderer->createDirLight(glm::vec3(.0f, -1.0f, .0f), glm::vec3(1.0f, .4f, 1.0f), 1.0f);
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

	//DirLight &dl = renderer->getDirLight(dirLightHandle);
	//dl.intensity = .5f + .5f* abs(sin(Time::now().asSeconds()));

	PointLight& pl = renderer->getPointLight(pointLightHandle);
	float sway = sin(Time::now().asSeconds()*.5f) * 20.0f * .16f;
	pl.position = glm::vec3(50.0f, 50.0f, 50.0f) * .16f;
	pl.position.x += sway;
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

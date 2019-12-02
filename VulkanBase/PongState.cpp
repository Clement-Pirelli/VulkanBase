#include "PongState.h"
#include "EntityManager.h"
#include "Entity.h"
#include <string>

#include "glfw3.h"
#include "PlayerInputComponent.h"
#include "PongMovementComponent.h"
#include "Transform.h"
#include "Camera.h"
#include "VertexBufferObject.h"
#include "Vertex.h"
#include "Singleton.h"
#include "Renderer.h"
#include "Utilities.h"


PongState::PongState(StateMachine *givenStateMachine, EntityManager *givenEntityManager) : State(givenStateMachine), entityManager(givenEntityManager)
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	cameraTransform = new Transform(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(.0f, .0f, 5.0f), glm::vec3(.0f, .0f, .0f));
	camera = new Camera(cameraTransform, 9.0f/6.0f, 90.0f, .01f, 200.0f);
	renderer->setCamera(camera);
	shaderHandle = renderer->createShader("shaders/normalfrag.spv", "shaders/normalvert.spv");
	transforms.resize(1000);
	modelHandles.resize(1000);
	std::vector<glm::vec4> colors;
	colors.resize(1000);


	for(int i = 0; i < 1000; i++)
	{
		float scaleScalar = (float)(Util::rand() % 5)*.025f;
		glm::vec3 scale(scaleScalar, scaleScalar, scaleScalar);
		glm::vec3 position((float)(Util::rand() % 100), (float)(Util::rand() % 100), (float)(Util::rand() % 20));
		position *= .16f;
		glm::vec3 rotation((float)(Util::rand() % 360), (float)(Util::rand() % 360), (float)(Util::rand() % 360));
		transforms[i] = new Transform(scale, position, rotation);
		colors[i] = glm::vec4((Util::rand() % 100) / 100.0f, (Util::rand() % 100) / 100.0f, (Util::rand() % 100) / 100.0f, 1.0f);
	}

	modelHandles = renderer->createModels(shaderHandle, transforms, "_assets/textures/robot.jpg", "_assets/meshes/robot.o", colors);

	PlayerInputComponent *input = new PlayerInputComponent(GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D);
	PongMovementComponent *movement = new PongMovementComponent(input);
	std::vector<Component *>components = { input, movement };
	entityManager->addEntity(new Entity(components, cameraTransform));
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
	entityManager->onUpdate(deltaTime);
	entityManager->onLateUpdate(deltaTime);
}

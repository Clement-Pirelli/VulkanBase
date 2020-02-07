#include "BoidController.h"
#include "Model.h"
#include "Singleton.h"

BoidController::BoidController()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	shader = renderer->createShader("shaders/frag.spv", "shaders/vert.spv");


	for(size_t i = 0; i < boidsAmount; i++)
	{

		glm::vec3 boidDirection = (-.5f + glm::vec3(float(rand()) / RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX));
		boidDirection = glm::normalize(boidDirection);
		boids[i].init(this, boidDirection, i);


		glm::vec3 boidPosition = glm::vec3(float(rand())/RAND_MAX, float(rand()) / RAND_MAX, float(rand()) / RAND_MAX)* boundariesDimensions;

		boidModels[i] = renderer->createModel(shader, Transform(glm::vec3(.15f,.15f,.15f), boidPosition, glm::vec3(.0f,.0f,.0f)), "_assets/textures/white.png", "_assets/meshes/boid.o", glm::vec4(1.0f,1.0f,1.0f,1.0f));
	}
}

BoidController::~BoidController()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	for (size_t i = 0; i < boidsAmount; i++)
	{
		renderer->destroyModel(boidModels[i]);
	}
}

void BoidController::update(float deltaTime)
{
	for (size_t i = 0; i < boidsAmount; i++) boids[i].update(deltaTime);

	for (size_t i = 0; i < boidsAmount; i++) boids[i].lateUpdate(deltaTime);
}

Transform &BoidController::getTransform(size_t boidIndex)
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	return renderer->getModel(boidModels[boidIndex]).transform;
}

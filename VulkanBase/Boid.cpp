#include "Boid.h"
#include "BoidController.h"
#include "glm.hpp"
#include "Renderer.h"

void Boid::init(BoidController *givenController, glm::vec3 givenDirection, size_t givenIndex)
{
	controller = givenController;
	myIndex = givenIndex;
	direction = givenDirection;
}

void Boid::update(float deltaTime)
{
	glm::vec3 avoidanceDirection(.0f);
	int avoidanceCount = 0;
	glm::vec3 directionDirection(.0f);
	int directionCount = 0;

	Transform &myTransform = controller->getTransform(myIndex);

	glm::vec3 boidsCenter = glm::vec3();
	int cohesionCount = 0;

	glm::vec3 currentPosition = myTransform.getLocalPosition();

	for(size_t i = 0; i < BoidController::boidsAmount; i++)
	{
		if (i == myIndex)
		{
			cohesionCount++;
			boidsCenter += myTransform.getLocalPosition();
			break;
		}

		glm::vec3 boidPosition = controller->getTransform(i).getLocalPosition();
		
		glm::vec3 toOtherBoid = boidPosition - currentPosition;
		float distance = glm::length(toOtherBoid);
		glm::vec3 directionTowards = toOtherBoid / distance;

		if(distance < BoidController::avoidanceRadius)
		{
			avoidanceCount++;
			avoidanceDirection += directionTowards / distance;
		}
		if(distance < BoidController::cohesionRadius)
		{
			cohesionCount++;
			boidsCenter += boidPosition;
		}
		if(distance < BoidController::directionRadius)
		{
			directionCount++;
			directionDirection += controller->boids[i].direction;
		}
	}

	glm::vec3 cohesionDirection = boidsCenter - currentPosition;
	glm::vec3 currentDirection = glm::vec3();

	if(cohesionCount > 0 && (abs(cohesionDirection.x >.0f) || abs(cohesionDirection.y > .0f) || abs(cohesionDirection.y > .0f)))
		currentDirection += cohesionDirection/float(cohesionCount)*BoidController::cohesionWeight;

	if (directionCount > 0)
		currentDirection += directionDirection/float(directionCount)*BoidController::directionWeight;

	if (avoidanceCount > 0)
		currentDirection -= avoidanceDirection/float(cohesionCount)*BoidController::avoidanceWeight;

	glm::vec3 toCenter = -currentPosition;
	currentDirection += toCenter*.01f*deltaTime;

	direction = glm::normalize(direction + currentDirection);

}

void Boid::lateUpdate(float deltaTime)
{
	Transform &myTransform = controller->getTransform(myIndex);
	myTransform.addLocalPosition(direction * deltaTime * BoidController::boidSpeed);
	myTransform.setLocalRotation(glm::quatLookAt(direction * glm::vec3(-1.0f,1.0f,1.0f), glm::normalize(glm::vec3(.0f,1.0f,.0f))));
}

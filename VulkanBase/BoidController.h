#pragma once
#include "Boid.h"
#include "Renderer.h"


class BoidController
{
	friend Boid;
public:

	BoidController();
	~BoidController();

	void update(float deltaTime);

private:

	static constexpr float boundariesDimensions = 8.0f;
	static constexpr size_t boidsAmount = 300;

	static constexpr float boidSpeed = 8.0f;
	static constexpr float avoidanceRadius = 1.0f;
	static constexpr float cohesionRadius = 3.0f;
	static constexpr float directionRadius = 6.0f;
	static constexpr float avoidanceWeight = 10.0f;
	static constexpr float cohesionWeight = 1.0f;
	static constexpr float directionWeight = 1.0f; //todo : make these non-const, let the user change them somewhere?

	Transform &getTransform(size_t boidIndex);


	Boid boids[boidsAmount];
	ModelHandle boidModels[boidsAmount];
	ShaderHandle shader = {};

};


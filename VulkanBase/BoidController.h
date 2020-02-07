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

	static constexpr float boundariesDimensions = 20.0f;
	static constexpr size_t boidsAmount = 100;

	static constexpr float boidSpeed = 4.0f;
	static constexpr float avoidanceRadius = .2f;
	static constexpr float cohesionRadius = 10.4f;
	static constexpr float directionRadius = 1.3f;
	static constexpr float avoidanceWeight = 1.0f;
	static constexpr float cohesionWeight = 1.0f;
	static constexpr float directionWeight = .4f; //todo : make these non-static, let the user change them somewhere?

	Transform &getTransform(size_t boidIndex);


	Boid boids[boidsAmount];
	ModelHandle boidModels[boidsAmount];
	ShaderHandle shader = {};

};


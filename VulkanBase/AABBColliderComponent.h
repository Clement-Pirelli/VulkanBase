#pragma once
#include "ColliderComponent.h"
#include "glm.hpp"
#include "Utilities.h"

class AABBColliderComponent :
	public ColliderComponent
{
public:
	AABBColliderComponent(unsigned int givenLayer, box givenBox);
	~AABBColliderComponent();

	box getAABB();

private:
	box aabb;
};


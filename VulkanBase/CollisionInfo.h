#pragma once

#ifndef COLLISIONINFO_DEFINED
#define COLLISIONINFO_DEFINED

#include "glm.hpp"
#include "Entity.h"

struct CollisionInfo
{
	glm::vec3 point;
	Entity *other;
};

#endif // !COLLISIONINFO_DEFINED
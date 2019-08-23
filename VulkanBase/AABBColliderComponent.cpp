#include "AABBColliderComponent.h"



AABBColliderComponent::AABBColliderComponent(unsigned int givenLayer, box givenBox) : ColliderComponent(givenLayer, AABB)
{
	aabb = givenBox;
}


AABBColliderComponent::~AABBColliderComponent(){}

box AABBColliderComponent::getAABB()
{
	return aabb;
}

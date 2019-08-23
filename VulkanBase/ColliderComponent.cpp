#include "ColliderComponent.h"
#include "Singleton.h"
#include "CollisionManager.h"


ColliderComponent::ColliderComponent(unsigned int givenLayer, COLLIDER_TYPE givenType) : layer(givenLayer), type(givenType){}


ColliderComponent::~ColliderComponent()
{
	Singleton<CollisionManager>::getInstance()->removeCollider(this);
}

unsigned int ColliderComponent::getLayer()
{
	return layer;
}

COLLIDER_TYPE ColliderComponent::getType()
{
	return type;
}

void ColliderComponent::onActive()
{
	Singleton<CollisionManager>::getInstance()->addCollider(this);
}

void ColliderComponent::onInactive()
{
	Singleton<CollisionManager>::getInstance()->removeCollider(this);
}

void ColliderComponent::onInit()
{
	Singleton<CollisionManager>::getInstance()->addCollider(this);
}

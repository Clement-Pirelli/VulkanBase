#include "Entity.h"
#include "Component.h"
#include "Transform.h"

Entity::Entity(std::vector<Component *> givenComponents, Transform *givenTransform)
{
	transform = givenTransform;
	components = givenComponents;
	for (Component *c : components) c->setOwner(this);
	for (Component *c : components) c->onInit();
}

Entity::Entity(Transform *givenTransform) : transform(givenTransform){}

Entity::Entity(Component *givenComponent, Transform *givenTransform) : transform(givenTransform)
{
	givenComponent->setOwner(this);
	givenComponent->onInit();
	components.push_back(givenComponent);
}

Entity::~Entity()
{
	for (Component *c : components)
	{
		delete c;
		c = nullptr;
	} 
	components.clear();

	delete transform;
	transform = nullptr;
}

void Entity::onUpdate(float deltaTime)
{
	for (Component *c : components)
	{
		c->onUpdate(deltaTime);
	}
}

void Entity::onLateUpdate(float deltaTime)
{
	for(Component *c : components)
	{
		c->onLateUpdate(deltaTime);
	}
}

void Entity::onActive()
{
	for (Component *c : components)
	{
		c->onActive();
	}
}

void Entity::onInactive()
{
	for (Component *c : components)
	{
		c->onInactive();
	}
}

void Entity::onCollision(Entity *other)
{
	for(Component *c : components)
	{
		c->onCollision(other);
	}
}

void Entity::addComponent(Component *givenComponent)
{
	givenComponent->setOwner(this);
	givenComponent->onInit();
	components.push_back(givenComponent);
}

Transform *Entity::getTransform()
{
	return transform;
}
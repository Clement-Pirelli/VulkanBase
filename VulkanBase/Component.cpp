#include "Component.h"
#include "Entity.h"

Component::~Component()
{
	owner = nullptr;
}

void Component::setOwner(Entity *entity)
{
	owner = entity;
}

Entity *Component::getOwner()
{
	return owner;
}

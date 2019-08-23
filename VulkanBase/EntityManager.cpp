#include "EntityManager.h"
#include "Entity.h"


EntityManager::EntityManager()
{
}


EntityManager::~EntityManager()
{
	for(unsigned int i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}
	entities.clear();
}

void EntityManager::addEntity(Entity *givenEntity)
{
	entities.push_back(givenEntity);
}

void EntityManager::removeEntity(Entity *givenEntity)
{
	size_t index = 0;
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i] == givenEntity)
		{
			index = i;
			break;
		}
	}
	entities.erase(entities.begin() + index);
}

void EntityManager::setEntityActive(Entity *givenEntity)
{
	size_t index = 0;
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i] == givenEntity)
		{
			return;
		}
	}
	addEntity(givenEntity);
	givenEntity->onActive();
}

void EntityManager::setEntityInactive(Entity *givenEntity)
{
	size_t index = 0;
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i] == givenEntity)
		{
			index = i;
			break;
		}
	}
	entities.erase(entities.begin() + index);
	givenEntity->onInactive();
}

void EntityManager::onUpdate(float deltaTime)
{
	for(Entity *e : entities)
	{
		e->onUpdate(deltaTime);
	}
}

void EntityManager::onLateUpdate(float deltaTime)
{
	for (Entity *e : entities)
	{
		e->onLateUpdate(deltaTime);
	}
}

void EntityManager::clearEntities()
{
	for (unsigned int i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}
	entities.clear();
}

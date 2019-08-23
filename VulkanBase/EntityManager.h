#pragma once
#include <vector>

class Entity;

class EntityManager
{
public:
	EntityManager();
	~EntityManager();

	void addEntity(Entity *givenEntity);
	void removeEntity(Entity *givenEntity);

	void setEntityActive(Entity *givenEntity);
	void setEntityInactive(Entity *givenEntity);

	void onUpdate(float deltaTime);
	void onLateUpdate(float deltaTime);

	void clearEntities();

private:
	std::vector<Entity*> entities;
};


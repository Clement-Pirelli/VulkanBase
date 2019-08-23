#pragma once
#ifndef ENTITY_INCLUDED
#define ENTITY_INCLUDED

#include <vector>
#include "Component.h"



class Transform;

class Entity
{
public:
	Entity(std::vector<Component *> givenComponents, Transform *givenTransform);
	Entity(Transform *givenTransform);
	Entity(Component *givenComponent, Transform *givenTransform);
	~Entity();

	void onUpdate(float deltaTime);
	void onLateUpdate(float deltaTime);
	void onActive();
	void onInactive();

	void onCollision(Entity *other);
	
	void addComponent(Component *givenComponent);

	template <class T>
	bool requestComponent(T* &componentPointer)
	{
		for (Component *c : components)
		{
			T* t = dynamic_cast<T*>(c);
			if (t != nullptr)
			{
				componentPointer = t;
				return true;
			}
		}
		return false;
	}

	Transform *getTransform();

private:
	//entity owns its transform and components
	Transform *transform;
	std::vector<Component *> components;
};

#endif
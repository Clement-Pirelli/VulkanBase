#pragma once
#ifndef COMPONENT_INCLUDED
#define COMPONENT_INCLUDED


class Entity;

class Component
{
public:
	Component(){}
	virtual ~Component();

	void setOwner(Entity *entity);
	Entity *getOwner();

	virtual void onUpdate(float deltaTime){}
	virtual void onLateUpdate(float deltaTime){}
	virtual void onInit(){}
	virtual void onInactive(){}
	virtual void onActive(){}
	virtual void onCollision(Entity *other){}

protected:
	Entity *owner;
};

#endif
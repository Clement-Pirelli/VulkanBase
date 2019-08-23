#pragma once
#include "Component.h"

enum COLLIDER_TYPE
{
	AABB = 1,
	SPHERE = 1 << 1
};

class ColliderComponent : public Component
{
public:
	ColliderComponent(unsigned int givenLayer, COLLIDER_TYPE givenType);
	virtual ~ColliderComponent();

	unsigned int getLayer();
	COLLIDER_TYPE getType();

	void onActive() override;
	void onInactive() override;
	void onInit() override;

protected:
	unsigned int layer = 0;
	COLLIDER_TYPE type;
private:
	ColliderComponent(){}
};


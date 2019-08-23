#include "CollisionManager.h"
#include "ColliderComponent.h"
#include "AABBColliderComponent.h"
#include "Entity.h"

CollisionManager::CollisionManager()
{
}


CollisionManager::~CollisionManager()
{
}

void CollisionManager::onUpdate()
{
	for(unsigned int i = 0; i < colliders.size(); i++)
	{
		for(unsigned int j = colliders.size()-1; j > i; j++)
		{
			bool collision = false;
			ColliderComponent *a = colliders[i], *b = colliders[j];
			int mask = a->getType() | b->getType();

			switch(mask)
			{
			case AABB:
				collision = AABBCollision(static_cast<AABBColliderComponent*>(a), static_cast<AABBColliderComponent*>(b));
				break;
			case AABB | SPHERE:
				//TODO
				break;
			case SPHERE:
				//TODO
				break;
			}

			if(collision)
			{
				Entity *aOwner = a->getOwner();
				Entity *bOwner = b->getOwner();

				aOwner->onCollision(bOwner);
				bOwner->onCollision(aOwner);
			}
		}
	}
}

void CollisionManager::addCollider(ColliderComponent *givenCollider)
{

	colliders.push_back(givenCollider);
}

void CollisionManager::removeCollider(ColliderComponent *givenCollider)
{
	size_t index = 0;
	for (size_t i = 0; i < colliders.size(); i++)
	{
		if (colliders[i] == givenCollider)
		{
			index = i;
			break;
		}
	}
	colliders.erase(colliders.begin() + index);
}

bool CollisionManager::AABBCollision(AABBColliderComponent *a, AABBColliderComponent *b)
{
	box aBox = a->getAABB();
	box bBox = b->getAABB();

	return 
		(
			(aBox.bottomLeftNear.x <= bBox.topRightFar.x && aBox.topRightFar.x >= bBox.bottomLeftNear.x) &&
			(aBox.bottomLeftNear.y <= bBox.topRightFar.y && aBox.topRightFar.y >= bBox.bottomLeftNear.y) &&
			(aBox.bottomLeftNear.z <= bBox.topRightFar.z && aBox.topRightFar.z >= bBox.bottomLeftNear.z)
		);
}

#pragma once
#include <vector>


class ColliderComponent;
class AABBColliderComponent;
struct collisionInfo;

class CollisionManager
{
public:
	CollisionManager();
	~CollisionManager();

	void onUpdate();
	void addCollider(ColliderComponent *givenCollider);
	void removeCollider(ColliderComponent *givenCollider);

private:
	static bool AABBCollision(AABBColliderComponent *a, AABBColliderComponent *b);
	std::vector<ColliderComponent *> colliders;
};


#pragma once
#ifndef TRANSFORM_DEFINED
#define TRANSFORM_DEFINED

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <vector>

class Transform
{
public:

	//constructs a transform with a given local scale, position and rotation, not parented
	Transform(glm::vec3 givenScale, glm::vec3 givenPosition, glm::vec3 givenRotation);
	//constructs a transform with identity local scale, position and rotation, not parented
	Transform();
	//constructs a transform with identity local scale, position and rotation, parented to a given transform
	Transform(Transform *givenParent);
	//constructs a transform with a given local scale, position and rotation, parented to a given transform
	Transform(Transform *givenParent, glm::vec3 givenScale, glm::vec3 givenPosition, glm::vec3 givenRotation);
	~Transform();

	glm::mat4 getGlobalTransform();
	glm::mat4 getLocalTransform();

	void setLocalScale(glm::vec3 givenScale);
	void setLocalPosition(glm::vec3 givenPosition);
	void setLocalRotation(glm::vec3 givenRotation);

	void addLocalScale(glm::vec3 givenScaleDelta);
	void addLocalPosition(glm::vec3 givenPositionDelta);
	void addLocalRotation(glm::vec3 givenRotationDelta);
	
	glm::vec3 getLocalScale();
	glm::vec3 getLocalPosition();
	glm::vec3 getLocalRotation();

	glm::vec3 getGlobalScale();
	glm::vec3 getGlobalPosition();
	glm::vec3 getGlobalRotation();

	//remove parent of current transform
	void unParent();
	
	//remove given child of current transform
	void removeChild(Transform *child);

	//add current child to the current transform
	void addChild(Transform *child);

	//sets the current transform's parent
	void setParent(Transform *parent);

	//removes all children of the current transform
	void removeChildren();

private:

	void updateLocalTransform();

	glm::vec3 scale;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::mat4 localTransform;
	Transform *parent;
	std::vector<Transform *> children;
};

#endif // !TRANSFORM_DEFINED


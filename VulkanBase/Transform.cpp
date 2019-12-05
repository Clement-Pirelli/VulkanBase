#include "Transform.h"

Transform::Transform(glm::vec3 givenScale, glm::vec3 givenPosition, glm::vec3 givenRotation)
{
	scale = givenScale;
	position = givenPosition;
	rotation = givenRotation;
	updateLocalTransform();
}

Transform::Transform()
{
	scale = glm::vec3(1.0f);
	rotation = glm::vec3(.0f);
	position = glm::vec3(.0f);
	updateLocalTransform();
}

Transform::Transform(Transform * givenParent)
{
	scale = glm::vec3(1.0f);
	parent = givenParent;
	parent->addChild(this);
	updateLocalTransform();
}

Transform::Transform(Transform * givenParent, glm::vec3 givenScale, glm::vec3 givenPosition, glm::vec3 givenRotation)
{
	scale = givenScale;
	position = givenPosition;
	rotation = givenRotation;
	setParent(givenParent);
	updateLocalTransform();
}

Transform::~Transform()
{
	removeChildren();
	if (parent != nullptr)
		parent->removeChild(this);
}

glm::mat4 Transform::getGlobalTransform() const
{
	return (parent != nullptr) ? (parent->getGlobalTransform() * localTransform) : (localTransform);
}

glm::mat4 Transform::getLocalTransform() const
{
	return localTransform;
}

void Transform::setLocalScale(glm::vec3 givenScale)
{
	scale = givenScale;
	updateLocalTransform();
}

void Transform::setLocalPosition(glm::vec3 givenPosition)
{
	position = givenPosition;
	updateLocalTransform();
}

void Transform::setLocalRotation(glm::vec3 givenRotation)
{
	rotation = glm::quat(givenRotation.x, glm::vec3(1.0f, .0f, .0f)) *
		glm::quat(givenRotation.y, glm::vec3(.0f, 1.0f, .0f)) *
		glm::quat(givenRotation.z, glm::vec3(.0f, .0f, 1.0f));
	updateLocalTransform();
}

void Transform::addLocalScale(glm::vec3 givenScaleDelta)
{
	scale += givenScaleDelta;
	updateLocalTransform();
}

void Transform::addLocalPosition(glm::vec3 givenPositionDelta)
{
	position += givenPositionDelta;
	updateLocalTransform();
}

void Transform::addLocalRotation(glm::vec3 givenRotationDelta)
{
	glm::quat rotateQuat = glm::quat(givenRotationDelta.x, glm::vec3(1.0f,.0f,.0f)) * 
		glm::quat(givenRotationDelta.y, glm::vec3(.0f,1.0f,.0f)) *
		glm::quat(givenRotationDelta.z, glm::vec3(.0f,.0f,1.0f));
	rotation *= rotateQuat;
	updateLocalTransform();
}

glm::vec3 Transform::getLocalScale() const
{
	return scale;
}

glm::vec3 Transform::getLocalPosition() const
{
	return position;
}

glm::vec3 Transform::getLocalRotation() const
{
	return glm::eulerAngles(rotation);
}

glm::vec3 Transform::getGlobalScale() const
{
	return (parent != nullptr) ? (parent->getGlobalScale() * scale) : (scale);
}

glm::vec3 Transform::getGlobalPosition() const
{
	return (parent != nullptr) ? (parent->getGlobalPosition() * position) : (position);
}

glm::vec3 Transform::getGlobalRotation() const
{
	return (parent != nullptr) ? (parent->getGlobalRotation() * getLocalRotation()) : (getLocalRotation());
}

glm::mat4 Transform::getLocalScaleMat() const
{
	return glm::scale(glm::mat4(1.0f), scale);
}

glm::mat4 Transform::getLocalPositionMat() const
{
	return glm::translate(glm::mat4(1.0f), position);
}

glm::mat4 Transform::getLocalRotationMat() const
{
	return glm::toMat4(rotation);
}

glm::mat4 Transform::getGlobalScaleMat() const
{
	return (parent != nullptr) ? (parent->getGlobalScaleMat() * getLocalScaleMat()) : (getLocalScaleMat());
}

glm::mat4 Transform::getGlobalPositionMat() const
{
	return (parent != nullptr) ? (parent->getGlobalPositionMat() * getLocalPositionMat()) : (getLocalPositionMat());
}

glm::mat4 Transform::getGlobalRotationMat() const
{
	return (parent != nullptr) ? (parent->getGlobalRotationMat() * getLocalRotationMat()) : (getLocalRotationMat());
}

void Transform::unParent()
{
	setParent(nullptr);
}

void Transform::removeChild(Transform * child)
{
	auto it = children.end();
	for (unsigned int i = 0; i < children.size(); i++)
	{
		if (children[i] == child)
		{
			it = children.begin() + i;
		}
	}

	if (it != children.end())
		children.erase(it);
}

void Transform::addChild(Transform *child)
{
	children.push_back(child);
}

void Transform::setParent(Transform * givenParent)
{
	if (parent != nullptr) parent->removeChild(this);
	if(givenParent != nullptr) givenParent->addChild(this);
	parent = givenParent;
}

void Transform::removeChildren()
{
	for (Transform *child : children)
	{
		child->unParent();
	}
	children.clear();
}

void Transform::updateLocalTransform()
{
	glm::mat4 rot = getLocalRotationMat();
	glm::mat4 sca = getLocalScaleMat();
	glm::mat4 pos = getLocalPositionMat();
	localTransform = pos * rot * sca;
}
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

glm::mat4 Transform::getGlobalTransform()
{
	return (parent != nullptr) ? (parent->getGlobalTransform() * localTransform) : (localTransform);
}

glm::mat4 Transform::getLocalTransform()
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
	rotation = givenRotation;
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
	rotation += givenRotationDelta;
	updateLocalTransform();
}

glm::vec3 Transform::getLocalScale()
{
	return scale;
}

glm::vec3 Transform::getLocalPosition()
{
	return position;
}

glm::vec3 Transform::getLocalRotation()
{
	return rotation;
}

glm::vec3 Transform::getGlobalScale()
{
	return (parent != nullptr) ? (parent->getGlobalScale() * scale) : (scale);
}

glm::vec3 Transform::getGlobalPosition()
{
	return (parent != nullptr) ? (parent->getGlobalPosition() * position) : (position);
}

glm::vec3 Transform::getGlobalRotation()
{
	return (parent != nullptr) ? (parent->getGlobalRotation() * rotation) : (rotation);
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
	glm::mat4 rot = glm::mat4(1.0f);
	rot = glm::rotate(rot, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	rot = glm::rotate(rot, rotation.y, glm::vec3(.0f, 1.0f, .0f));
	rot = glm::rotate(rot, rotation.z, glm::vec3(.0f, .0f, 1.0f));
	glm::mat4 sca = glm::scale(glm::mat4(1.0f), scale);
	glm::mat4 tra = glm::translate(glm::mat4(1.0f), position);
	localTransform = tra * rot * sca;
}
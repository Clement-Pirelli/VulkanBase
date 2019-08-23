#include "MeshComponent.h"
#include "Model.h"
#include "Entity.h"

MeshComponent::MeshComponent(const char *givenTexturePath, const char *givenVboPath) : texturePath(givenTexturePath), vboPath(givenVboPath){}


MeshComponent::~MeshComponent()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeModel(model);
	delete model;
	model = nullptr;
}

void MeshComponent::onInit()
{
	model = new Model(owner->getTransform(), texturePath, vboPath);
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->addModel(model);
}

void MeshComponent::onInactive()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeModel(model);
}

void MeshComponent::onActive()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	uniformDataCreationInfo creationInfo = renderer->getUniformDataCreationInfo();
	model->recreateUniformData(creationInfo.device, creationInfo.physicalDevice, creationInfo.swapChainData, creationInfo.descriptorSetLayout);
	renderer->addModel(model);
}

void MeshComponent::setColor(glm::vec4 givenColor)
{
	model->setColor(givenColor);
}

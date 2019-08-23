#include "UITextComponent.h"
#include "VertexBufferObject.h"
#include "Renderer.h"
#include "Model.h"
#include "Singleton.h"
#include "Entity.h"

UITextComponent::UITextComponent(const char *givenFontTexturePath, const char *givenText, glm::vec4 givenColor) : fontTexturePath(givenFontTexturePath)
{
	vbo = new VertexBufferObject();
	text = givenText;
	color = givenColor;
	setVBO();
}

UITextComponent::~UITextComponent()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeUIModel(model);
	delete model;
	vbo->cleanup(renderer->getVBOCreationInfo().device);
	delete vbo;
}

void UITextComponent::onInit()
{
	model = new Model(owner->getTransform(), fontTexturePath, vbo);
	setColor(color);
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->addUIModel(model);
}

void UITextComponent::onActive()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	uniformDataCreationInfo creationInfo = renderer->getUniformDataCreationInfo();
	model->recreateUniformData(creationInfo.device, creationInfo.physicalDevice, creationInfo.swapChainData, creationInfo.descriptorSetLayout);
	renderer->addUIModel(model);
}

void UITextComponent::onInactive()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeUIModel(model);
}

void UITextComponent::setColor(glm::vec4 givenColor)
{
	model->setColor(givenColor);
}

void UITextComponent::setText(const char * givenText)
{
	text = givenText;
	resetVBO();
}

void UITextComponent::resetVBO()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeUIModel(model);
	vbo->cleanup(renderer->getVBOCreationInfo().device);
	setVBO();
	uniformDataCreationInfo creationInfo = renderer->getUniformDataCreationInfo();
	model->recreateUniformData(creationInfo.device, creationInfo.physicalDevice, creationInfo.swapChainData, creationInfo.descriptorSetLayout);
	renderer->addUIModel(model);
}

void UITextComponent::setVBO()
{
	float xScale = 1.0f / 16.0f;
	float yScale = 1.0f / 16.0f;

	const char *currentChar = text;
	unsigned int letterCount = 0;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	float textLength = (float)std::strlen(text);

	while (*currentChar != '\0')
	{
		Vertex v1, v2, v3, v4;
		glm::vec2 uv = { (float)((*currentChar) % 16) * xScale, (float)((*currentChar) / 16) * yScale };
		glm::vec2 st = { uv.x + xScale, uv.y + yScale };

		float letterCountFloat = (float)letterCount;// -textLength / 2.0f;

		v1.pos = glm::vec3(letterCountFloat, 1.0f, .0f);
		v1.texCoord = uv;

		v2.pos = glm::vec3(letterCountFloat, .0f, .0f);
		v2.texCoord = glm::vec2(uv.x, st.y);

		v3.pos = glm::vec3(letterCountFloat + 1.0f, .0f, .0f);
		v3.texCoord = st;

		v4.pos = glm::vec3(letterCountFloat + 1.0f, 1.0f, .0f);
		v4.texCoord = glm::vec2(st.x, uv.y);

		/*
		v2--v3
		| /	|
		v1--v4
		*/
		vertices.push_back(v1);
		vertices.push_back(v2);
		vertices.push_back(v3);
		vertices.push_back(v4);

		/*
		1 - 2
		| / |
		0 - 3
		*/

		indices.push_back(letterCount * 4 + 2);
		indices.push_back(letterCount * 4 + 0);
		indices.push_back(letterCount * 4 + 1);
		indices.push_back(letterCount * 4 + 2);
		indices.push_back(letterCount * 4 + 3);
		indices.push_back(letterCount * 4 + 0);

		letterCount++;
		currentChar = text + letterCount;
	}

	Renderer *renderer = Singleton<Renderer>::getInstance();
	VBOCreationInfo creationInfo = renderer->getVBOCreationInfo();

	vbo->setVertices(vertices);
	vbo->setIndices(indices);
	vbo->createVertexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
	vbo->createIndexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
}

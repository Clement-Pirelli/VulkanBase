#include "UIText.h"
#include "VertexBufferObject.h"
#include "Renderer.h"
#include "Model.h"
#include "Singleton.h"

UIText::UIText(const char *fontTexturePath, const char *text, Transform *givenTransform, glm::vec3 givenColor)
{
	transform = givenTransform;
	vbo = new VertexBufferObject();
	model = new Model(givenTransform, fontTexturePath, vbo);
	setText(text, givenColor);
}

UIText::~UIText()
{
	//removeModel cleans up uniform data
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeUIModel(model);
	delete model;
	vbo->cleanup(renderer->getVBOCreationInfo().device);
	delete vbo;
}

void UIText::setText(const char *givenText, glm::vec3 givenColor)
{
	float xScale = 1.0f / 16.0f;
	float yScale = 1.0f / 16.0f;
	
	const char *text = givenText;
	unsigned int letterCount = 0;
	
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	float textLength = (float)std::strlen(givenText);
	
	while(*text != '\0')
	{
		Vertex v1, v2, v3, v4;
		glm::vec2 uv = { (float)((*text) % 16) * xScale, (float)((*text) / 16) * yScale };
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

		indices.push_back(letterCount*4+2);
		indices.push_back(letterCount*4+0);
		indices.push_back(letterCount*4+1);
		indices.push_back(letterCount*4+2);
		indices.push_back(letterCount*4+3);
		indices.push_back(letterCount*4+0);

		letterCount++;
		text = givenText+letterCount;
	}

	Renderer *renderer = Singleton<Renderer>::getInstance();
	VBOCreationInfo creationInfo = renderer->getVBOCreationInfo();

	vbo->setVertices(vertices);
	vbo->setIndices(indices);
	vbo->createVertexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
	vbo->createIndexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);

	renderer->addUIModel(model);
}
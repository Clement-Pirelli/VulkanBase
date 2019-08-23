#include "PongPaddleMeshComponent.h"
#include "Entity.h"
#include "Model.h"
#include "VertexBufferObject.h"
#include "Vertex.h"
#include "CameraComponent.h"
#include "Renderer.h"
#include "Singleton.h"

PongPaddleMeshComponent::PongPaddleMeshComponent(CameraComponent *givenCamera) : camera(givenCamera)
{
}


PongPaddleMeshComponent::~PongPaddleMeshComponent()
{
	Renderer *renderer = Singleton<Renderer>::getInstance();
	renderer->removeModel(model);
	vbo->cleanup(renderer->getVBOCreationInfo().device);
	delete model;
	camera = nullptr;
}

void PongPaddleMeshComponent::onInit()
{
	setModel();
	Singleton<Renderer>::getInstance()->addModel(model);
}

void PongPaddleMeshComponent::onActive()
{
	Singleton<Renderer>::getInstance()->addModel(model);
}

void PongPaddleMeshComponent::onInactive()
{
	Singleton<Renderer>::getInstance()->removeModel(model);
}


inline void getUnitCube(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
	glm::vec3 positions[8] =
	{
		glm::vec3(-1.0f,-1.0f,-1.0f),
		glm::vec3(-1.0f,-1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),
		glm::vec3( 1.0f, 1.0f,-1.0f),
		glm::vec3(-1.0f, 1.0f,-1.0f),
		glm::vec3( 1.0f,-1.0f, 1.0f),
		glm::vec3( 1.0f,-1.0f,-1.0f),
		glm::vec3( 1.0f, 1.0f, 1.0f),
	};

	uint32_t ind[36] =
	{
		0,	1,	2,
		0,	2,	4,
		5,  7,	1,
		7,	2,	1,	
		1,	6,	5,	
		1,	0,	6,
		4,	3,	6,
		0,	4,	6,
		7,	3,	2,
		3,	4,	2,
		6,	3,	5,
		3,	7,	5 
	};

	vertices.resize(8);
	indices.resize(36);


	for(unsigned int i = 0; i < 8; i++)
	{
		vertices[i].pos = positions[i];
		vertices[i].texCoord = glm::vec2(.0f,.0f);
	}

	for(unsigned int i = 0; i < 36; i++)
	{
		indices[i] = ind[i];
	}
}

void PongPaddleMeshComponent::setModel()
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	getUnitCube(vertices, indices);

	float position = camera->getZNear() + .2f;
	float height = tan(glm::radians(camera->getDegFOV())*.5f)*position*2.0f;
	float width = height * (1920.0f / 1080.0f);

	for (size_t i = 0; i < vertices.size(); i++)
	{
		vertices[i].pos.x *= width;
		vertices[i].pos.y *= height;
		vertices[i].pos.z *= .2f;
	}



	vbo = new VertexBufferObject();
	vbo->setVertices(vertices);
	vbo->setIndices(indices);

	VBOCreationInfo creationInfo = Singleton<Renderer>::getInstance()->getVBOCreationInfo();
	vbo->createVertexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);
	vbo->createIndexBuffer(creationInfo.device, creationInfo.physicalDevice, creationInfo.commandPool, creationInfo.graphicsQueue);

	model = new Model(owner->getTransform(), "_assets/textures/white.png", vbo);
}
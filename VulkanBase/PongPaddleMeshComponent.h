#pragma once
#include "Component.h"

class Model;
class VertexBufferObject;
class CameraComponent;

class PongPaddleMeshComponent : public Component
{
public:
	PongPaddleMeshComponent(CameraComponent *givenCamera);
	~PongPaddleMeshComponent();

	void onInit() override;
	void onActive() override;
	void onInactive() override;

private:

	void setModel();
	Model *model = nullptr;
	VertexBufferObject *vbo = nullptr;
	CameraComponent *camera = nullptr;
};


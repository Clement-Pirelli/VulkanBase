#pragma once
#include "Component.h"
#include "Delegate.h"
#include "glm.hpp"

enum DIRECTION
{
	UP,
	DOWN,
	LEFT,
	RIGHT,
	COUNT
};

class PlayerInputComponent : public Component
{
public:
	PlayerInputComponent(int upKey, int downKey, int leftKey, int rightKey);
	~PlayerInputComponent();

	void onUp(delegateInfo &info);
	void onDown(delegateInfo &info);
	void onLeft(delegateInfo &info);
	void onRight(delegateInfo &info);

	void onUpdate(float deltaTime) override;
	void onLateUpdate(float deltaTime) override;

	glm::vec2 getInputDirection();

	glm::dvec2 getMouseDelta();
	static void setMouseSensitivity(float givenSensitivity);

private:
	bool input[DIRECTION::COUNT];
	int inputKeys[DIRECTION::COUNT];
	Delegate inputCallback[DIRECTION::COUNT];

	glm::dvec2 lastMousePos = glm::vec2(.0f);
	glm::dvec2 currentMousePos = glm::vec2(.0f);
	static float mouseSensitivity;
};


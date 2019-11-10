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

struct InputInfo;

class PlayerInputComponent : public Component
{
public:
	PlayerInputComponent(int upKey, int downKey, int leftKey, int rightKey);
	~PlayerInputComponent();

	void onUp(InputInfo &info);
	void onDown(InputInfo &info);
	void onLeft(InputInfo &info);
	void onRight(InputInfo &info);

	void onUpdate(float deltaTime) override;
	void onLateUpdate(float deltaTime) override;
	void onActive() override;
	void onInactive() override;

	glm::vec2 getInputDirection();

	glm::dvec2 getMouseDelta();
	static void setMouseSensitivity(float givenSensitivity);

private:
	bool input[DIRECTION::COUNT];
	int inputKeys[DIRECTION::COUNT];
	Delegate<InputInfo> inputCallback[DIRECTION::COUNT];

	glm::dvec2 lastMousePos = glm::vec2(.0f);
	glm::dvec2 currentMousePos = glm::vec2(.0f);
	static float mouseSensitivity;
};


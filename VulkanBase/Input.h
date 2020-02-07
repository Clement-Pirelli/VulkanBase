#pragma once
#include "Delegate.h"
#include "glm.hpp"
#include "Camera.h"

enum DIRECTION
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN,
	COUNT
};

struct InputInfo;

class Input
{
public:
	Input(int forwardKey, int backwardKey, int leftKey, int rightKey, int upKey, int downKey);
	~Input();

	void onKey(InputInfo &info);

	bool isGoingFast() { return goingFast; }
	void onUpdate();
	void onLateUpdate();

	glm::vec3 getInputDirection();

	glm::dvec2 getMouseDelta();
	static void setMouseSensitivity(float givenSensitivity);

private:
	bool input[DIRECTION::COUNT];
	int inputKeys[DIRECTION::COUNT];
	Delegate<InputInfo> inputCallback;

	glm::dvec2 lastMousePos = glm::vec2(.0f);
	glm::dvec2 currentMousePos = glm::vec2(.0f);
	static float mouseSensitivity;
	bool goingFast = false;
};


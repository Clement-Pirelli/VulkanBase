#include "Input.h"
#include "InputManager.h"
#include "glfw3.h"

float Input::mouseSensitivity = .01f;

Input::Input(int forwardKey, int backwardKey, int leftKey, int rightKey, int upKey, int downKey)
{
	for(unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		input[i] = false;
	}

	inputKeys[DIRECTION::FORWARD] = forwardKey;
	inputKeys[DIRECTION::BACKWARD] = backwardKey;
	inputKeys[DIRECTION::LEFT] = leftKey;
	inputKeys[DIRECTION::RIGHT] = rightKey;
	inputKeys[DIRECTION::UP] = upKey;
	inputKeys[DIRECTION::DOWN] = downKey;

	inputCallback = Delegate<InputInfo>::makeDelegate<Input, &Input::onKey>(this);

	InputManager::addKeyboardCallback(&inputCallback);

	lastMousePos = InputManager::getMousePosition();
	currentMousePos = InputManager::getMousePosition();
}


Input::~Input()
{
	for (unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		InputManager::removeKeyboardCallback(inputKeys[i], &inputCallback);
	}

	for (unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		input[i] = false;
	}
}

void Input::onKey(InputInfo & info)
{
	bool key_state = (info.state == INPUT_STATE::REPEATED || info.state == INPUT_STATE::PRESSED);
	for(int i = 0; i < DIRECTION::COUNT; i++)
	{
		if(info.key == inputKeys[i])
		{
			input[i] = key_state;
		}
	}
	goingFast = info.mods == GLFW_MOD_SHIFT;
}

void Input::onUpdate()
{
	currentMousePos = InputManager::getMousePosition();
}

void Input::onLateUpdate()
{
	lastMousePos = currentMousePos;
}

glm::vec3 Input::getInputDirection()
{
	glm::vec3 returnVec = glm::vec3(.0f);
	returnVec += glm::vec3(.0f,.0f,1.0f) * (float)(input[DIRECTION::BACKWARD] - input[DIRECTION::FORWARD]);
	returnVec += glm::vec3(1.0f,.0f,.0f) * (float)(input[DIRECTION::RIGHT] - input[DIRECTION::LEFT]);
	returnVec += glm::vec3(.0f,1.0f,.0f) * (float)(input[DIRECTION::UP] - input[DIRECTION::DOWN]);
	if (abs(returnVec.x) > .001f || abs(returnVec.y) > .001f || abs(returnVec.z) > .001f) 
	{
		returnVec = glm::normalize(returnVec);
	}
	return returnVec;
}

glm::dvec2 Input::getMouseDelta()
{
	return (currentMousePos-lastMousePos)*(double)mouseSensitivity;
}

void Input::setMouseSensitivity(float givenSensitivity)
{
	mouseSensitivity = givenSensitivity;
}

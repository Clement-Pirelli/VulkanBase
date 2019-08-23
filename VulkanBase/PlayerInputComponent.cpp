#include "PlayerInputComponent.h"
#include "InputManager.h"

float PlayerInputComponent::mouseSensitivity = 1.0f;

PlayerInputComponent::PlayerInputComponent(int upKey, int downKey, int leftKey, int rightKey)
{
	for(unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		input[i] = false;
	}

	inputKeys[DIRECTION::UP] = upKey;
	inputCallback[DIRECTION::UP] = Delegate::makeDelegate<PlayerInputComponent, &PlayerInputComponent::onUp>(this);

	inputKeys[DIRECTION::DOWN] = downKey;
	inputCallback[DIRECTION::DOWN] = Delegate::makeDelegate<PlayerInputComponent, &PlayerInputComponent::onDown>(this);
	
	inputKeys[DIRECTION::LEFT] = leftKey;
	inputCallback[DIRECTION::LEFT] = Delegate::makeDelegate<PlayerInputComponent, &PlayerInputComponent::onLeft>(this);

	inputKeys[DIRECTION::RIGHT] = rightKey;
	inputCallback[DIRECTION::RIGHT] = Delegate::makeDelegate<PlayerInputComponent, &PlayerInputComponent::onRight>(this);


	for (unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		InputManager::addKeyboardCallback(inputKeys[i], inputCallback+i);
	}

	lastMousePos = InputManager::getMousePosition();
	currentMousePos = InputManager::getMousePosition();
}


PlayerInputComponent::~PlayerInputComponent()
{
	for (unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		InputManager::removeKeyboardCallback(inputKeys[i], &inputCallback[i]);
	}

	for (unsigned int i = 0; i < DIRECTION::COUNT; i++)
	{
		input[i] = false;
	}
}

void PlayerInputComponent::onUp(delegateInfo &info)
{
	input[DIRECTION::UP] = (info.ix == INPUT_STATE::REPEATED || info.ix == INPUT_STATE::PRESSED);
}

void PlayerInputComponent::onDown(delegateInfo &info)
{
	input[DIRECTION::DOWN] = (info.ix == INPUT_STATE::REPEATED || info.ix == INPUT_STATE::PRESSED);
}

void PlayerInputComponent::onLeft(delegateInfo &info)
{
	input[DIRECTION::LEFT] = (info.ix == INPUT_STATE::REPEATED || info.ix == INPUT_STATE::PRESSED);
}

void PlayerInputComponent::onRight(delegateInfo &info)
{
	input[DIRECTION::RIGHT] = (info.ix == INPUT_STATE::REPEATED || info.ix == INPUT_STATE::PRESSED);
}

void PlayerInputComponent::onUpdate(float deltaTime)
{
	currentMousePos = InputManager::getMousePosition();
}

void PlayerInputComponent::onLateUpdate(float deltaTime)
{
	lastMousePos = currentMousePos;
}

glm::vec2 PlayerInputComponent::getInputDirection()
{
	glm::vec2 returnVec = glm::vec2(.0f);
	if (input[DIRECTION::UP])
		returnVec.y += 1.0f;
	if (input[DIRECTION::DOWN])
		returnVec.y -= 1.0f;
	if (input[DIRECTION::RIGHT])
		returnVec.x += 1.0f;
	if (input[DIRECTION::LEFT])
		returnVec.x -= 1.0f;
	if (abs(returnVec.x) > .001f && abs(returnVec.y) > .001f) 
	{
		returnVec = glm::normalize(returnVec);
	}
	return returnVec;
}

glm::dvec2 PlayerInputComponent::getMouseDelta()
{
	return (currentMousePos-lastMousePos)*(double)mouseSensitivity;
}

void PlayerInputComponent::setMouseSensitivity(float givenSensitivity)
{
	mouseSensitivity = givenSensitivity;
}

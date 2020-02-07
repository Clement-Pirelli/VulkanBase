#include "InputManager.h"
#include <glfw3.h>


DelegateVector InputManager::keyboardCallbacks = DelegateVector();
std::unordered_map<int, DelegateVector> InputManager::mouseCallbacks = std::unordered_map<int, DelegateVector>();
GLFWwindow *InputManager::window = nullptr;

static INPUT_STATE glfwToEnum(int glfwAction)
{
	switch (glfwAction)
	{
	case GLFW_PRESS:
		return INPUT_STATE::PRESSED;
	case GLFW_RELEASE:
		return INPUT_STATE::RELEASED;
	case GLFW_REPEAT:
		return INPUT_STATE::REPEATED;
	}
	return INPUT_STATE::UNDEFINED;
}


void InputManager::addButtonCallback(int givenButton, Delegate<InputInfo> *givenDelegate)
{
	if (mouseCallbacks.count(givenButton) == 0)
	{
		std::vector<Delegate<InputInfo>*> v = std::vector<Delegate<InputInfo>*>();
		v.push_back(givenDelegate);
		mouseCallbacks.emplace(std::pair<int, std::vector<Delegate<InputInfo>*>>(givenButton, v));
	} else
	{
		auto callbacks = mouseCallbacks[givenButton];
		callbacks.push_back(givenDelegate);
	}
}

void InputManager::addKeyboardCallback(Delegate<InputInfo> *givenDelegate)
{
	keyboardCallbacks.push_back(givenDelegate);
}

void InputManager::removeButtonCallback(int givenButton, Delegate<InputInfo> *givenDelegate)
{
	if (mouseCallbacks.count(givenButton) > 0)
	{
		auto callbacks = mouseCallbacks[givenButton];
		size_t index = 0;
		for (size_t i = 0; i < callbacks.size(); i++)
		{
			if (callbacks[i] == givenDelegate)
			{
				index = i;
				break;
			}
		}
		callbacks.erase(callbacks.begin() + index);
	}
}

void InputManager::removeKeyboardCallback(int givenKey, Delegate<InputInfo> *givenDelegate)
{
	for (size_t i = 0; i < keyboardCallbacks.size(); i++)
	{
		if (keyboardCallbacks[i] == givenDelegate)
		{
			keyboardCallbacks.erase(keyboardCallbacks.begin() + i);
			return;
		}
	}
}

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	if (mouseCallbacks.count(button) > 0)
	{
		auto callbacks = mouseCallbacks[button];
		InputInfo info;
		info.state = glfwToEnum(action);
		info.mods = mods;
		for (Delegate<InputInfo> *callback : callbacks)
		{
			(*callback)(info);
		}
	}
}

void InputManager::keyboardKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	for(auto callback : keyboardCallbacks)
	{
		InputInfo info;
		info.state = glfwToEnum(action);
		info.mods = mods;
		info.key = key;
		(*callback)(info);
	}
}

glm::dvec2 InputManager::getMousePosition()
{
	double x = 0, y = 0;
	glfwGetCursorPos(window, &x, &y);

	return glm::dvec2(x, y);
}

void InputManager::setWindow(GLFWwindow * givenWindow)
{
	window = givenWindow;
}

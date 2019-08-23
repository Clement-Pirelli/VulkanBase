#include "InputManager.h"
#include <glfw3.h>
#include "Delegate.h"


std::unordered_map<int, DelegateVector> InputManager::keyboardCallbacks	=	std::unordered_map<int, DelegateVector>();
std::unordered_map<int, DelegateVector> InputManager::mouseCallbacks	=	std::unordered_map<int, DelegateVector>();
GLFWwindow *InputManager::window = nullptr;

static int glfwToEnum(int glfwAction)
{
	switch(glfwAction)
	{
	case GLFW_PRESS:
		return INPUT_STATE::PRESSED;
	case GLFW_RELEASE:
		return INPUT_STATE::RELEASED;
	case GLFW_REPEAT:
		return INPUT_STATE::REPEATED;
	}
	return -1;
}


void InputManager::addButtonCallback(int givenButton, Delegate *givenDelegate)
{
	if (mouseCallbacks.count(givenButton) == 0)
	{
		std::vector<Delegate*> v = std::vector<Delegate*>();
		v.push_back(givenDelegate);
		mouseCallbacks.emplace(std::pair<int, std::vector<Delegate*>>(givenButton, v));
	} else 
	{
		auto callbacks = mouseCallbacks[givenButton];
		callbacks.push_back(givenDelegate);
	}
}

void InputManager::addKeyboardCallback(int givenKey, Delegate *givenDelegate)
{
	if (keyboardCallbacks.count(givenKey) == 0)
	{
		std::vector<Delegate*> v = std::vector<Delegate*>();
		v.push_back(givenDelegate);
		keyboardCallbacks.emplace(std::pair<int, std::vector<Delegate*>>(givenKey, v));
	} else
	{
		auto callbacks = keyboardCallbacks[givenKey];
		callbacks.push_back(givenDelegate);
	}
}

void InputManager::removeButtonCallback(int givenButton, Delegate *givenDelegate)
{
	if(mouseCallbacks.count(givenButton) > 0)
	{
		auto callbacks = mouseCallbacks[givenButton];
		size_t index = 0;
		for(size_t i = 0; i < callbacks.size();i++)
		{
			if(callbacks[i] == givenDelegate)
			{
				index = i;
				break;
			}
		}
		callbacks.erase(callbacks.begin() + index);
	}
}

void InputManager::removeKeyboardCallback(int givenKey, Delegate *givenDelegate)
{
	if (keyboardCallbacks.count(givenKey) > 0)
	{
		auto callbacks = keyboardCallbacks[givenKey];
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

void InputManager::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
	if(mouseCallbacks.count(button) > 0)
	{
		auto callbacks = mouseCallbacks[button];
		delegateInfo info;
		info.ix = glfwToEnum(action);
		for(Delegate *callback : callbacks)
		{
			(*callback)(info);
		}
	}
}

void InputManager::keyboardKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (keyboardCallbacks.count(key) > 0)
	{
		auto callbacks = keyboardCallbacks[key];
		delegateInfo info;
		info.ix = glfwToEnum(action);
		info.iy = mods;
		for (Delegate *callback : callbacks)
		{
			(*callback)(info);
		}
	}
}

glm::dvec2 InputManager::getMousePosition()
{
	double x = 0, y = 0;
	glfwGetCursorPos(window, &x, &y);

	return glm::dvec2(x,y);
}

void InputManager::setWindow(GLFWwindow * givenWindow)
{
	window = givenWindow;
}

#pragma once
#include <unordered_map>
#include <vector>
#include "glm.hpp"
#include "Delegate.h"


//other parts of the program don't need to know of GLFW to know what to do with the input
enum INPUT_STATE
{
	PRESSED,
	RELEASED,
	REPEATED,
	UNDEFINED
};

struct InputInfo
{
	INPUT_STATE state;
	int mods;
};



struct GLFWwindow;
typedef std::vector<Delegate<InputInfo>*> DelegateVector;

class InputManager
{
public:

	static void addButtonCallback(int givenButton, Delegate<InputInfo> *givenDelegate);
	static void addKeyboardCallback(int givenKey, Delegate<InputInfo> *givenDelegate);
	static void removeButtonCallback(int givenButton, Delegate<InputInfo> *givenDelegate);
	static void removeKeyboardCallback(int givenKey, Delegate<InputInfo> *givenDelegate);
	static void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
	static void keyboardKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static glm::dvec2 getMousePosition();
	static void setWindow(GLFWwindow *givenWindow);

private:

	InputManager(){};
	~InputManager(){};
	static GLFWwindow *window;
	static std::unordered_map<int, DelegateVector> keyboardCallbacks;
	static std::unordered_map<int, DelegateVector> mouseCallbacks;
};
#pragma once

#include <unordered_map>
#include <vector>
#include "glm.hpp"

struct GLFWwindow;
class Delegate;
typedef std::vector<Delegate*> DelegateVector;

//other parts of the program don't need to know of GLFW to know what to do with the input
enum INPUT_STATE
{
	PRESSED,
	RELEASED,
	REPEATED,
};

class InputManager
{
public:

	static void addButtonCallback(int givenButton, Delegate *givenDelegate);
	static void addKeyboardCallback(int givenKey, Delegate *givenDelegate);
	static void removeButtonCallback(int givenButton, Delegate *givenDelegate);
	static void removeKeyboardCallback(int givenKey, Delegate *givenDelegate);
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
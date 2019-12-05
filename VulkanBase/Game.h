#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "Delegate.h"

class InputManager;
class EntityManager;
class StateMachine;
class Renderer;
struct InputInfo;

class Game
{
public:
	Game();
	~Game();
	int run();


	static constexpr unsigned int WIDTH = 900;
	static constexpr unsigned int HEIGHT = 600;


private:


	Renderer *renderer;
	InputManager *inputManager;
	StateMachine *stateMachine;
	

	GLFWwindow *window;

	void initWindow();
	void onKeyPressed(InputInfo &info);
	bool shouldQuit = false;
	Delegate<InputInfo> onEscapeDelegate;
};


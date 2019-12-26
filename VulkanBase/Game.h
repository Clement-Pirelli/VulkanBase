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


	static constexpr unsigned int WINDOWED_WIDTH = 1920/3;
	static constexpr unsigned int WINDOWED_HEIGHT = 1080/3;


	static constexpr unsigned int FULLSCREEN_WIDTH = 1920;
	static constexpr unsigned int FULLSCREEN_HEIGHT = 1080;


private:


	Renderer *renderer;
	InputManager *inputManager;
	StateMachine *stateMachine;
	

	GLFWwindow *window;

	void initWindow();
	void onKeyPressed(InputInfo &info);
	bool shouldQuit = false;
	bool fullScreen = false;
	bool initComplete = false;
	Delegate<InputInfo> onEscapeDelegate;
};


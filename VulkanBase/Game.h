#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "Delegate.h"

class Renderer;
class AudioManager;
class InputManager;
class EntityManager;
class CollisionManager;
class StateMachine;
struct InputInfo;


class Game
{
public:
	Game();
	~Game();
	int run();

private:
	AudioManager* audioManager;
	Renderer *renderer;
	InputManager *inputManager;
	EntityManager *entityManager;
	CollisionManager *collisionManager;
	StateMachine *stateMachine;
	
	const unsigned int WIDTH = 800;
	const unsigned int HEIGHT = 500;
	GLFWwindow *window;
	void initWindow();
	void onEscape(InputInfo &info);
	bool shouldQuit = false;
	Delegate<InputInfo> onEscapeDelegate;
};


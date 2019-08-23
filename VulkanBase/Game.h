#pragma once
#include <vector>
#include <GLFW/glfw3.h>
#include "Delegate.h"

class Renderer;
class Model;
class Transform;
class Camera;
class UIText;
class AudioManager;
class InputManager;
class EntityManager;
class UITextComponent;
class CollisionManager;
class StateMachine;


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
	Camera *camera;
	
	const unsigned int WIDTH = 800;
	const unsigned int HEIGHT = 500;
	GLFWwindow *window;
	void initWindow();
	void onEscape(delegateInfo &info);
	bool shouldQuit = false;
	Delegate onEscapeDelegate;
};


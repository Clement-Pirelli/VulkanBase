#include "Game.h"
#include "Renderer.h"
#include <iostream>
#include "Utilities.h"
#include "glm.hpp"
#include "Time.h"
#include "Singleton.h"
#include "InputManager.h"
#include "StateMachine.h"
#include "PongState.h"


Game::Game()
{
	initWindow();

	renderer = new Renderer(window, glm::vec2(HEIGHT, WIDTH));
	Singleton<Renderer>::setInstance(renderer);

	stateMachine = new StateMachine();
	PongState *p = new PongState(stateMachine);
	stateMachine->setFirstState(p);
}

Game::~Game()
{
	delete stateMachine;
	delete renderer;

	glfwDestroyWindow(window);
	glfwTerminate();
}

int Game::run()
{
	const auto startTime = Time::now();
	float deltaTime = .0f;
	float lastTime = .0f;
	float time = .0f;


	//60 FPS
	constexpr float frameTimer = 1.0f/60.0f;
	float timeSinceLastFrame = .0f;

	try {
		while (!shouldQuit) {
			auto currentTime = Time::now();
			time = (currentTime - startTime).asSeconds();
			deltaTime = time - lastTime;

			glfwPollEvents();
			if (glfwWindowShouldClose(window)) break;

			timeSinceLastFrame += deltaTime;
			if(timeSinceLastFrame >= frameTimer)
			{
				stateMachine->onUpdate(timeSinceLastFrame);
				renderer->render();
				timeSinceLastFrame = .0f;
			}

			lastTime = time;
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void Game::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	//reminder for windowed fullscreen : 
	//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	//
	//mode->width;
	//mode->height;

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowAspectRatio(window, 9, 6);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, Renderer::framebufferResizeCallback);
	
	InputManager::setWindow(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetKeyCallback(window, &InputManager::keyboardKeyCallback);
	glfwSetMouseButtonCallback(window, &InputManager::mouseButtonCallback);

	onEscapeDelegate = Delegate<InputInfo>::makeDelegate<Game, &Game::onKeyPressed>(this);
	InputManager::addKeyboardCallback(&onEscapeDelegate);
}

void Game::onKeyPressed(InputInfo &info)
{
	if(info.key == GLFW_KEY_ESCAPE && info.state == INPUT_STATE::RELEASED)
		shouldQuit = true;
}

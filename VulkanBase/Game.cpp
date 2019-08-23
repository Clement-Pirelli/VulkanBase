#include "Game.h"
#include "Renderer.h"
#include <iostream>
#include "Utilities.h"
#include "glm.hpp"
#include <chrono>
#include "Transform.h"
#include <string>
#include "AudioManager.h"
#include "Singleton.h"
#include "InputManager.h"

#include "EntityManager.h"
#include "CollisionManager.h"
#include "StateMachine.h"
#include "PongState.h"

Game::Game()
{
	initWindow();
	
	audioManager = new AudioManager();
	Singleton<AudioManager>::setInstance(audioManager);

	renderer = new Renderer(window);
	Singleton<Renderer>::setInstance(renderer);

	collisionManager = new CollisionManager();
	Singleton<CollisionManager>::setInstance(collisionManager);

	entityManager = new EntityManager();

	stateMachine = new StateMachine();
	PongState *p = new PongState(stateMachine, entityManager);
	stateMachine->setFirstState(p);
}

Game::~Game()
{
	delete entityManager;
	delete audioManager;
	delete collisionManager;
	delete stateMachine;
	delete renderer;
	delete camera;

	glfwDestroyWindow(window);
	glfwTerminate();
}

int Game::run()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	float deltaTime = .0f;
	float lastTime = .0f;
	float time = .0f;
	try {
		while (!shouldQuit) {
			auto currentTime = std::chrono::high_resolution_clock::now();
			time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
			deltaTime = time - lastTime;

			glfwPollEvents();
			if (glfwWindowShouldClose(window)) break;

			stateMachine->onUpdate(deltaTime);

			renderer->render();

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

	//for windowed fullscreen : 
	//const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	//
	//mode->width;
	//mode->height;
	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowAspectRatio(window, 1920, 1080);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, Renderer::framebufferResizeCallback);
	
	InputManager::setWindow(window);
	glfwSetKeyCallback(window, &InputManager::keyboardKeyCallback);
	glfwSetMouseButtonCallback(window, &InputManager::mouseButtonCallback);


	onEscapeDelegate = Delegate::makeDelegate<Game, &Game::onEscape>(this);
	InputManager::addKeyboardCallback(GLFW_KEY_ESCAPE, &onEscapeDelegate);

}

void Game::onEscape(delegateInfo &info)
{
	shouldQuit = true;
}

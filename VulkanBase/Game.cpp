#include "Game.h"
#include "Renderer.h"
#include <iostream>
#include "glm.hpp"
#include "Time.h"
#include "Singleton.h"
#include "InputManager.h"
#include "StateMachine.h"
#include "ExampleState.h"


Game::Game()
{
	initWindow();

	renderer = new Renderer(window, glm::vec2(WINDOWED_HEIGHT, WINDOWED_WIDTH));
	Singleton<Renderer>::setInstance(renderer);

	stateMachine = new StateMachine();
	ExampleState *p = new ExampleState(stateMachine);
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
			timeSinceLastFrame += deltaTime;
			
			if(timeSinceLastFrame >= frameTimer)
			{
				glfwPollEvents();
				if (glfwWindowShouldClose(window)) break;
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

	window = glfwCreateWindow(WINDOWED_WIDTH, WINDOWED_HEIGHT, "Vulkan", nullptr, nullptr);
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
	if(info.state == INPUT_STATE::RELEASED)
	{
		switch(info.key)
		{
		case GLFW_KEY_ESCAPE:
		{
			shouldQuit = true;
		} break;
		case GLFW_KEY_R:
		{
			fullScreen = !fullScreen;

			int newWidth = fullScreen ? FULLSCREEN_WIDTH : WINDOWED_WIDTH;
			int newHeight = fullScreen ? FULLSCREEN_HEIGHT : WINDOWED_HEIGHT;

			glfwSetWindowSize(window, newWidth, newHeight);

			const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			int newWindowPosX = mode->width / 2 - newWidth/2;
			int newWindowPosY = mode->height / 2 - newHeight / 2;
			glfwSetWindowPos(window, newWindowPosX, newWindowPosY);

		}break;
		}
	}
}

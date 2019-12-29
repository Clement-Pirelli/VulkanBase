#include "Game.h"
#include "Renderer.h"
#include <iostream>
#include "glm.hpp"
#include "Time.h"
#include "Singleton.h"
#include <string>

Game::Game()
{
	std::cout << "Welcome to the screen shader renderer! Please specify the path of the screen shader you wish to render: ";
	std::string shaderPath = "";
	std::cin >> shaderPath;
	std::cout << "\nNow please enter the width of the end image: ";
	int width = 0;
	std::cin >> width;
	int height = 0;
	std::cout << "\nNow please enter the height of the end image: ";
	std::cin >> height;
	std::cout << "\nThank you! Now please enter the name you wish to give to the end image: ";
	std::string imageName = "";
	std::cin >> imageName;
	std::cout << "\nThank you. I will now start rendering\n";

	renderer = new Renderer(glm::ivec2(width, height), shaderPath.c_str());
	renderer->render();
	renderer->writeImageToFile(imageName.c_str());

	std::cout << "\nRendering completed! Thank you for your time!";
}

Game::~Game()
{
	delete renderer;
}
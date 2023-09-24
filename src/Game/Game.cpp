#include "Game.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>

Game::Game() {
	isRunning = false;
	registry = std::make_unique<Registry>();
	Logger::Log("Game construct called.");
}

Game::~Game() {
	Logger::Log("Game destructor called");
}

void Game::Initialize() {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		Logger::Err("Error initializing SDL");
		return;
	}

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	// set to constant (dont use displaymode dims) to use as logical dimensions
	windowWidth = 800; 
	windowHeight = 600; 
	// windowWidth = displayMode.w;
	// windowHeight = displayMode.h;

	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth,
		windowHeight,
		SDL_WINDOW_BORDERLESS
	);
	if (!window) {
		Logger::Err("Error creating SDL window.");
		return;
	}

	renderer = SDL_CreateRenderer(
		window, 
		-1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC // use both if possible
	);
	if (!renderer) {
		Logger::Err("Error creating SDL renderer.");
		return;
	}
	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	isRunning = true;
}

void Game::Setup() {
	// TODO
	Entity tank = registry->CreateEntity();
	Entity truck = registry->CreateEntity();
	// tank.addComponent<TransformComponent>();
	// tank.addComponent<BoxColliderComponent>();
	// tank.addComponent<SpriteComponent>("./path/to/png");
}

void Game::Update() {
	// if too fast, waste time till reach desired frame length aka MILLISECS_PER_FRAME
	// only works if update is fast, this is not frame-governed loop
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
		SDL_Delay(timeToWait);
	}

	// double dt = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

	// Store the now previous frame time
	millisecsPreviousFrame = SDL_GetTicks();

	// TODO
	// MovementSystem.Update();
	// CollisionSystem.Update();
	// DamageSystem.Update();
}

void Game::ProcessInput() {
	SDL_Event sdlEvent; // struct
	while (SDL_PollEvent(&sdlEvent)) {
		switch(sdlEvent.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_KEYDOWN:
				if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
					isRunning = false;
				}
				break;
		}
	}
}

void Game::Render() {
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	// TODO
	// Render game objects

	SDL_RenderPresent(renderer); // paints window
}

void Game::Run() {
	Setup();
	while (isRunning) {
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Destroy() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}
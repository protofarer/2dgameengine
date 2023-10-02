#include "Game.h"
#include "LevelLoader.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../Systems/MovementSystem.h"
#include "../Systems/RenderSystem.h"
#include "../Systems/AnimationSystem.h"
#include "../Systems/CollisionSystem.h"
#include "../Systems/DamageSystem.h"
#include "../Systems/KeyboardControlSystem.h"
#include "../Systems/CameraMovementSystem.h"
#include "../Systems/ProjectileEmitSystem.h"
#include "../Systems/ProjectileLifecycleSystem.h"
#include "../Systems/RenderTextSystem.h"
#include "../Systems/RenderHealthBarSystem.h"
#include "../Systems/RenderGUISystem.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>
#include <imgui/imgui_impl_sdl.h>
#include <vector>

// set to constant (dont use displaymode dims) to use as logical dimensions
int Game::logicalWidth = 800;
int Game::logicalHeight = 600;
int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;
double Game::entityTweak = 2.0;
double Game::tileTweak = 3.5;

Game::Game() {
	isRunning = false;
	isDebug = false;
	registry = std::make_unique<Registry>();
	assetStore = std::make_unique<AssetStore>();
	eventBus = std::make_unique<EventBus>();
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

	if (TTF_Init() != 0) {
		Logger::Err("Error initializing SDL TTF");
		return;
	}
	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);
	windowWidth = displayMode.w;
	windowHeight = displayMode.h;
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
	renderer = SDL_CreateRenderer(window, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC // use both if possible
	);
	if (!renderer) {
		Logger::Err("Error creating SDL renderer.");
		return;
	}

	// Init ImGui context
	ImGui::CreateContext();
	ImGuiSDL::Initialize(renderer, windowWidth, windowHeight);

	SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);

	// Initialize camera view with entire screen area
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;

	isRunning = true;
}

void Game::Setup() {
	// Add systems that need to be processed
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();
	registry->AddSystem<CollisionSystem>();
	registry->AddSystem<DamageSystem>();
	registry->AddSystem<KeyboardControlSystem>();
	registry->AddSystem<CameraMovementSystem>();
	registry->AddSystem<ProjectileEmitSystem>();
	registry->AddSystem<ProjectileLifecycleSystem>();
	registry->AddSystem<RenderTextSystem>();
	registry->AddSystem<RenderHealthBarSystem>();
	registry->AddSystem<RenderGUISystem>();

	lua.open_libraries(sol::lib::base, sol::lib::math);
	LevelLoader loader; // why not pass pointer to registry/assetSt/renderer in constructor?
	loader.LoadLevel(lua, registry, assetStore, renderer, 1);
}

void Game::ProcessInput() {
	SDL_Event sdlEvent; // struct
	while (SDL_PollEvent(&sdlEvent)) {
		// ImGui SDL input
		ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
		ImGuiIO& io = ImGui::GetIO();
		int mouseX, mouseY;
		const int buttons = SDL_GetMouseState(&mouseX, &mouseY);
		io.MousePos = ImVec2(mouseX, mouseY);
		io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

		// Handle core SDL (close window, key pressed, etc)
		switch(sdlEvent.type) {
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_KEYDOWN:
				switch(sdlEvent.key.keysym.sym) {
					case SDLK_ESCAPE:
						isRunning = false;
						break;
					case SDLK_d:
						isDebug = !isDebug;
						break;
					default:
						eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
						break;
				}
				break;
		}
	}
}

void Game::Update() {
	// if too fast, waste time till reach desired frame length aka MILLISECS_PER_FRAME
	// only works if update is fast, this is not frame-governed loop
	int timeToWait = MILLISECS_PER_FRAME - (SDL_GetTicks() - millisecsPreviousFrame);
	if (timeToWait > 0 && timeToWait <= MILLISECS_PER_FRAME) {
		SDL_Delay(timeToWait);
	}

	double dt = (SDL_GetTicks() - millisecsPreviousFrame) / 1000.0;

	// Store the now previous frame time
	millisecsPreviousFrame = SDL_GetTicks();

	// Reset all event handlers for current frame
	eventBus->Reset();

	// Perform subscription of the events for all systems
	// Frame by frame binding
	registry->GetSystem<DamageSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<ProjectileEmitSystem>().SubscribeToEvents(eventBus);
	registry->GetSystem<MovementSystem>().SubscribeToEvents(eventBus);

	// Update registry to process entities pending add/delete
	registry->Update();

	// Invoke all systems that update
	registry->GetSystem<MovementSystem>().Update(dt);
	registry->GetSystem<AnimationSystem>().Update();
	registry->GetSystem<CollisionSystem>().Update(eventBus);
	registry->GetSystem<CameraMovementSystem>().Update(camera);
	registry->GetSystem<ProjectileEmitSystem>().Update();
	registry->GetSystem<ProjectileLifecycleSystem>().Update();
}

void Game::Render() {
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
	registry->GetSystem<RenderTextSystem>().Update(renderer, assetStore, camera);
	registry->GetSystem<RenderHealthBarSystem>().Update(renderer, assetStore, camera);
	if (isDebug) {
		registry->GetSystem<CollisionSystem>().Render(renderer, camera);
		registry->GetSystem<RenderGUISystem>().Update(registry);
	}

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
	ImGuiSDL::Deinitialize();
	ImGui::DestroyContext();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

#include "Game.h"
#include "../Logger/Logger.h"
#include "../ECS/ECS.h"
#include "../ECS/Components/TransformComponent.h"
#include "../ECS/Components/RigidBodyComponent.h"
#include "../ECS/Components/SpriteComponent.h"
#include "../ECS/Components/AnimationComponent.h"
#include "../ECS/Components/BoxColliderComponent.h"
#include "../ECS/Components/CameraFollowComponent.h"
#include "../ECS/Systems/MovementSystem.h"
#include "../ECS/Systems/RenderSystem.h"
#include "../ECS/Systems/AnimationSystem.h"
#include "../ECS/Systems/CollisionSystem.h"
#include "../ECS/Systems/DamageSystem.h"
#include "../ECS/Systems/KeyboardControlSystem.h"
#include "../ECS/Systems/CameraMovementSystem.h"
#include "../ECS/Components/SpriteComponent.h"
#include "../ECS/Components/KeyboardControlComponent.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

// set to constant (dont use displaymode dims) to use as logical dimensions
int Game::logicalWidth = 800;
int Game::logicalHeight = 600;
int Game::windowWidth;
int Game::windowHeight;
int Game::mapWidth;
int Game::mapHeight;
double Game::entityTweak = 2.0;
double Game::tileTweak = 4.0;

Game::Game() {
	isRunning = false;
	isModeDebug = false;
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

	SDL_DisplayMode displayMode;
	SDL_GetCurrentDisplayMode(0, &displayMode);

	windowWidth = displayMode.w;
	windowHeight = displayMode.h;
	std::cout << "winWid " << windowWidth << " winHeight " << windowHeight <<"\n";


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

	// Initialize camera view with entire screen area
	camera.x = 0;
	camera.y = 0;
	camera.w = windowWidth;
	camera.h = windowHeight;

	isRunning = true;
}

void Game::LoadLevel(int level) {
	// Add systems that need to be processed
	registry->AddSystem<MovementSystem>();
	registry->AddSystem<RenderSystem>();
	registry->AddSystem<AnimationSystem>();
	registry->AddSystem<CollisionSystem>();
	registry->AddSystem<DamageSystem>();
	registry->AddSystem<KeyboardControlSystem>();
	registry->AddSystem<CameraMovementSystem>();

	// Add assets to asset store
	assetStore->AddTexture(renderer, "tank-image", "./assets/images/tank-panther-right.png");
	assetStore->AddTexture(renderer, "truck-image", "./assets/images/truck-ford-right.png");
	assetStore->AddTexture(renderer, "tilemap-image", "./assets/tilemaps/jungle.png");
	assetStore->AddTexture(renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
	assetStore->AddTexture(renderer, "radar-image", "./assets/images/radar.png");

	// 1. read map -> 2d array of srcImage indices
	std::vector<std::vector<int>> matrix = ReadMatrixFromFile("./assets/tilemaps/jungle.map", windowWidth, windowHeight);

	// 2. create tilemap entities, locate them to game world coordinates
	CreateTileMapEntities(matrix);

	double vel = 1000;
	Entity chopper = registry->CreateEntity();
	chopper.AddComponent<TransformComponent>(glm::vec2(10.0, 10.0), glm::vec2(entityTweak, entityTweak), 0.0);
	chopper.AddComponent<RigidBodyComponent>(glm::vec2(vel, 0.0));
	chopper.AddComponent<SpriteComponent>("chopper-image", 32, 32, 2);
	chopper.AddComponent<AnimationComponent>(2, 15, true);
	chopper.AddComponent<KeyboardControlComponent>(
		glm::vec2(0, -vel), 
		glm::vec2(vel, 0), 
		glm::vec2(0, vel), 
		glm::vec2(-vel, 0)
	);
	chopper.AddComponent<CameraFollowComponent>();

	Entity radar = registry->CreateEntity();
	radar.AddComponent<TransformComponent>(glm::vec2(windowWidth - 128, 32), glm::vec2(2.0, 2.0), 0.0);
	radar.AddComponent<SpriteComponent>("radar-image", 64, 64, 2, true);
	radar.AddComponent<AnimationComponent>(8, 5, true);

	Entity tank = registry->CreateEntity();
	tank.AddComponent<TransformComponent>(glm::vec2(100.0, 100.0), glm::vec2(entityTweak, entityTweak), 0.0);
	tank.AddComponent<RigidBodyComponent>(glm::vec2(100.0, 0.0));
	tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
	tank.AddComponent<BoxColliderComponent>(32, 32);

	// Entity truck = registry->CreateEntity();
	// truck.AddComponent<TransformComponent>(glm::vec2(10.0, 100.0), glm::vec2(entityTweak, entityTweak), 0.0);
	// truck.AddComponent<RigidBodyComponent>(glm::vec2(100.0, 0.0));
	// truck.AddComponent<SpriteComponent>("truck-image", 32, 32, 1);
	// truck.AddComponent<BoxColliderComponent>(32, 32);

	// Entity tank2 = registry->CreateEntity();
	// tank2.AddComponent<TransformComponent>(glm::vec2(300.0, 700.0), glm::vec2(entityTweak, entityTweak), 0.0);
	// tank2.AddComponent<RigidBodyComponent>(glm::vec2(-100.0, 0.0));
	// tank2.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
	// tank2.AddComponent<BoxColliderComponent>(32, 32);

	// Entity truck2 = registry->CreateEntity();
	// truck2.AddComponent<TransformComponent>(glm::vec2(100.0, 400.0), glm::vec2(entityTweak, entityTweak), 0.0);
	// truck2.AddComponent<RigidBodyComponent>(glm::vec2(100.0, 0.0));
	// truck2.AddComponent<SpriteComponent>("truck-image", 32, 32, 1);
	// truck2.AddComponent<BoxColliderComponent>(32, 32);
}

void Game::Setup() {
	LoadLevel(1);
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

	// Update registry to process entities pending add/delete
	registry->Update();

	// Invoke all systems that update
	registry->GetSystem<MovementSystem>().Update(dt);
	registry->GetSystem<AnimationSystem>().Update();
	registry->GetSystem<CollisionSystem>().Update(eventBus);
	registry->GetSystem<CameraMovementSystem>().Update(camera);
}

void Game::Render() {
	SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
	SDL_RenderClear(renderer);

	registry->GetSystem<RenderSystem>().Update(renderer, assetStore, camera);
	if (isModeDebug) {
		registry->GetSystem<CollisionSystem>().Render(renderer, camera);
	}

	SDL_RenderPresent(renderer); // paints window
}

void Game::ProcessInput() {
	SDL_Event sdlEvent; // struct
	while (SDL_PollEvent(&sdlEvent)) {
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
						isModeDebug = !isModeDebug;
						break;
					default:
						eventBus->EmitEvent<KeyPressedEvent>(sdlEvent.key.keysym.sym);
						break;
				}
				break;
		}
	}
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

void Game::CreateTileMapEntities(std::vector<std::vector<int>>& matrix) {
	const int n_map_rows = matrix.size();
	const int n_map_cols = matrix[0].size();

	int tileSize = 32;

	const int srcTilemapWidthPx = 320;
	const int srcTilemapHeightPx = 96;

	const int srcTileWidth = static_cast<int>(srcTilemapWidthPx / 10.0); // looked at image, 10 cols
	const int srcTileHeight = static_cast<int>(srcTilemapHeightPx / 3.0); // looked at image, 3 rows

	// const float dstTileWidth = static_cast<float>(windowWidth) / n_map_cols;
	// const float dstTileHeight = static_cast<float>(windowHeight) / n_map_rows;

	// const float scaleX = static_cast<float>(dstTileWidth) * 1.05 / srcTileWidth;
	// const float scaleY = static_cast<float>(dstTileHeight) / srcTileHeight;

	mapWidth = n_map_cols * tileSize * tileTweak;
	mapHeight = n_map_rows * tileSize * tileTweak;
	std::cout << "map W/H: " << mapWidth << "/" << mapHeight << "\n";

	// 2. iterate through map, each (col, row) index corresponds to some column width = windowWidth / maxCols, row height = windowHeight / maxRows
	// 2.a create entity -> specify transform, specify sprite (set srcRect x y)
	for (int y = 0; y < n_map_rows; y++) {
		for (int x = 0; x < n_map_cols; x++) {
			int img_no = matrix[y][x];
			int srcRow = img_no / 10;
			int srcCol = img_no % 10;
			int srcX = srcTileWidth * srcCol;
			int srcY = srcTileHeight * srcRow;
			int posX = tileSize * tileTweak * x;
			int posY = tileSize * tileTweak * y;
			Entity tile = registry->CreateEntity();
			tile.AddComponent<TransformComponent>(glm::vec2(posX, posY), glm::vec2(tileTweak, tileTweak), 0.0);
			tile.AddComponent<SpriteComponent>("tilemap-image", 32, 32, 0, false, srcX, srcY);
		}
	}
}


std::vector<std::vector<int>> Game::ReadMatrixFromFile(const std::string& filename, int windowWidth, int windowHeight) {
	// ? CSDR read n_rows, n_cols from file itself
	int mapNumRows = 20;
	int mapNumCols = 25;

	std::vector<std::vector<int>> matrix(mapNumRows, std::vector<int>(mapNumCols, 0));

	std::ifstream file(filename);
	if (!file) {
		std::cerr << "Error: Could not open file." << std::endl;
		return matrix; // Return an empty matrix on error
	}

    std::string line;
    int row = 0;

    while (std::getline(file, line) && row < mapNumRows) {
        std::istringstream iss(line);
        int col = 0;
        std::string token;

        while (std::getline(iss, token, ',')) {
            if (col < mapNumCols) {
                try {
                    matrix[row][col] = std::stoi(token); // Convert the token to an integer
                } catch (const std::invalid_argument& e) {
                    // Handle invalid integers if needed
                    std::cerr << "Invalid integer: " << token << std::endl;
                }
            }
            col++;
        }
        row++;
    }

    file.close();
    return matrix;
}

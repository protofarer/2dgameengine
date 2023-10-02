#pragma once

#include <SDL2/SDL.h>
#include <sol/sol.hpp>
#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../AssetStore/AssetStore.h"

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
	private:
		bool isRunning;
		bool isDebug;
		int millisecsPreviousFrame;
		SDL_Window* window;
		SDL_Renderer* renderer;
		SDL_Rect camera;
		sol::state lua;

		std::unique_ptr<Registry> registry;
		std::unique_ptr<AssetStore> assetStore;
		std::unique_ptr<EventBus> eventBus;

		std::vector<std::vector<int>> ReadMatrixFromFile(
			const std::string& filename, int windowWidth, int windowHeight);
		void CreateTileMapEntities(std::vector<std::vector<int>>& matrix);

	public:
		Game();
		~Game();
		void Initialize();
		void Setup();
		void LoadLevel(int level);
		void Run();
		void ProcessInput();
		void Update();
		void Render();
		void Destroy();

		static int logicalWidth;
		static int logicalHeight;
		static int windowWidth;
		static int windowHeight;
		static int mapWidth;
		static int mapHeight;
		static double entityTweak;
		static double tileTweak;
};
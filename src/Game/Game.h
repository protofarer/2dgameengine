#pragma once

#include <SDL2/SDL.h>
#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"

const int FPS = 60;
const int MILLISECS_PER_FRAME = 1000 / FPS;

class Game {
	private:
		bool isRunning;
		bool isModeDebug;
		int millisecsPreviousFrame;
		SDL_Window* window;
		SDL_Renderer* renderer;

		std::unique_ptr<Registry> registry;
		std::unique_ptr<AssetStore> assetStore;
		std::vector<std::vector<int>> ReadMatrixFromFile(
			const std::string& filename, int windowWidth, int windowHeight);
		void CreateTileMapping(std::vector<std::vector<int>>& matrix);

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

		int windowWidth;
		int windowHeight;
};
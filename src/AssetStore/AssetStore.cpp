#include "./AssetStore.h"
#include "../Logger/Logger.h"
#include "SDL2/SDL_image.h"

AssetStore::AssetStore() {
	Logger::Log("AssetStore constructor called.");
}

AssetStore::~AssetStore() {
	ClearAssets(); // in case forget to clear assets explicitly
	Logger::Log("AssetStore destructor called.");
}

void AssetStore::ClearAssets() {
	for (auto texture: textures) {
		SDL_DestroyTexture(texture.second); // dealloc textures in memory
	}
	textures.clear(); // only clears memory
}

void AssetStore::AddTexture(SDL_Renderer* renderer, const std::string& assetId, const std::string& filePath) {
	SDL_Surface* surface = IMG_Load(filePath.c_str());
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);

	// add texture to map
	textures.emplace(assetId, texture);

	Logger::Log("New texture added to asset store with id = " + assetId);
}

SDL_Texture* AssetStore::GetTexture(const std::string& assetId) {
	return textures[assetId];
}
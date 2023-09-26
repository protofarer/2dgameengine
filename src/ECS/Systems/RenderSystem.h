#pragma once

#include "../ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../../AssetStore/AssetStore.h"
#include <SDL2/SDL.h>

class RenderSystem: public System {
	public:
		RenderSystem() {
			RequireComponent<TransformComponent>();
		}

		void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore) {
			for (auto entity: GetSystemEntities()) {
				// Update entity sys based on velocity for every frame
				auto& transform = entity.GetComponent<TransformComponent>();
				const auto& sprite = entity.GetComponent<SpriteComponent>();

				// Set source rect of original sprite texture
				SDL_Rect srcRect = sprite.srcRect;

				// Set the dest rect with the x,y pos to be rendered
				SDL_Rect dstRect = {
					static_cast<int>(transform.position.x),
					static_cast<int>(transform.position.y),
					static_cast<int>(sprite.width * transform.scale.x), 
					static_cast<int>(sprite.height * transform.scale.y)
				};

				SDL_RenderCopyEx(
					renderer, 
					assetStore->GetTexture(sprite.assetId),
					&srcRect, &dstRect, 
					transform.rotation, NULL,
					SDL_FLIP_NONE
				);

				// Draw PNG texture
			}
		}
};
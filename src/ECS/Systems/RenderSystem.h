#pragma once

#include "../ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <SDL2/SDL.h>

class RenderSystem: public System {
	public:
		RenderSystem() {
			RequireComponent<TransformComponent>();
		}

		void Update(SDL_Renderer* renderer) {
			for (auto entity: GetSystemEntities()) {
			// Update entity sys based on velocity for every frame
				auto& transform = entity.GetComponent<TransformComponent>();
				const auto& sprite = entity.GetComponent<SpriteComponent>();

				SDL_Rect objRect = {
					static_cast<int>(transform.position.x),
					static_cast<int>(transform.position.y),
					sprite.width,
					sprite.height
				};
				SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
				SDL_RenderFillRect(renderer, &objRect);
			}
		}
};
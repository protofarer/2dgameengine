#pragma once

#include "../ECS/ECS.h"
#include "../AssetStore/AssetStore.h"
#include "../Components/HealthComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include <SDL2/SDL.h>

class RenderHealthBarSystem: public System {
	public:
		RenderHealthBarSystem() {
			RequireComponent<HealthComponent>();
			RequireComponent<TransformComponent>();
			RequireComponent<SpriteComponent>();
		}

		void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, const SDL_Rect& camera) {
			for (auto entity: GetSystemEntities()) {
				const auto health = entity.GetComponent<HealthComponent>();
				const auto transform = entity.GetComponent<TransformComponent>();
				const auto sprite = entity.GetComponent<SpriteComponent>();

				SDL_Color healthBarColor = { 0, 255, 0 };

				int hp = health.healthPercentage;
				if (hp >= 33 && hp < 66) {
					healthBarColor = { 255, 255, 0 };
				} else if (hp < 33) {
					healthBarColor = { 255, 0, 0 };
				}

				SDL_Surface* surface = TTF_RenderText_Blended(
					assetStore->GetFont("arial-font-15"), 
					std::to_string(health.healthPercentage).c_str(), 
					healthBarColor
				);
				SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
				SDL_FreeSurface(surface);

				int labelWidth = 0;
				int labelHeight = 0;

				SDL_QueryTexture(texture, NULL, NULL, &labelWidth, &labelHeight);

				double labelPosX = (transform.position.x) + (sprite.width * transform.scale.x / 2) - 15 - camera.x;
				double labelPosY = (transform.position.y) - 25 - camera.y;
				SDL_Rect dstRectLabel = {
					static_cast<int>(labelPosX),
					static_cast<int>(labelPosY),
					labelWidth,
					labelHeight
				};
				SDL_RenderCopy(renderer, texture, NULL, &dstRectLabel);

				SDL_DestroyTexture(texture);

				int barWidth = sprite.width * transform.scale.x;
				int barHeight = sprite.height / 5;
				double healthBarPosX = (transform.position.x) - camera.x;
				double healthBarPosY = (transform.position.y) - 5 - camera.y;

				SDL_Rect healthBarRectangle = {
					static_cast<int>(healthBarPosX),
					static_cast<int>(healthBarPosY),
					static_cast<int>(barWidth * hp / 100.0),
					static_cast<int>(barHeight)
				};
				SDL_SetRenderDrawColor(renderer, healthBarColor.r, healthBarColor.g, healthBarColor.b, 255);
				SDL_RenderFillRect(renderer, &healthBarRectangle);
			}
		}
};
#pragma once

#include "../ECS/ECS.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"
#include "../AssetStore/AssetStore.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <algorithm>

class RenderSystem: public System {
	public:
		RenderSystem() {
			RequireComponent<TransformComponent>();
		}

		void Update(SDL_Renderer* renderer, std::unique_ptr<AssetStore>& assetStore, SDL_Rect camera) {
			// Create vector with both sprite and transform component of each entity
			struct RenderableEntity {
				TransformComponent transformComponent;
				SpriteComponent spriteComponent;
			};
			std::vector<RenderableEntity> renderableEntities;
			for (auto entity: GetSystemEntities()) {
				RenderableEntity renderableEntity;
				renderableEntity.spriteComponent = entity.GetComponent<SpriteComponent>();
				renderableEntity.transformComponent = entity.GetComponent<TransformComponent>();
				bool isEntityOutsideCameraView = (
					renderableEntity.transformComponent.position.x + (renderableEntity.transformComponent.scale.x * renderableEntity.spriteComponent.width) < camera.x ||
					renderableEntity.transformComponent.position.x > camera.x + camera.w ||
					renderableEntity.transformComponent.position.y + (renderableEntity.transformComponent.scale.y * renderableEntity.spriteComponent.height) < camera.y ||
					renderableEntity.transformComponent.position.y > camera.y + camera.h
				);
				// except fixed sprites
				if (isEntityOutsideCameraView && !renderableEntity.spriteComponent.isFixed) {
					continue;
				}
				renderableEntities.emplace_back(renderableEntity);
			}

			// Sort entities of system by z index
			std::sort(renderableEntities.begin(), renderableEntities.end(), [](const RenderableEntity a, const RenderableEntity b) {
				return a.spriteComponent.zIndex < b.spriteComponent.zIndex;
			});


			// my sort by z index
			// std::vector<Entity> entities = GetSystemEntities();
			// std::sort(entities.begin(), entities.end(), [](const Entity a, const Entity b) {
			// 	return a.GetComponent<SpriteComponent>().zIndex < b.GetComponent<SpriteComponent>().zIndex;
			// });

			for (auto entity: renderableEntities) {
			// for (auto entity: entities) {
				// Update entity sys based on velocity for every frame
				auto& transform = entity.transformComponent;
				const auto& sprite = entity.spriteComponent;

				// my sort
				// auto& transform = entity.GetComponent<TransformComponent>();
				// const auto& sprite = entity.GetComponent<SpriteComponent>();

				// Set source rect of original sprite texture
				SDL_Rect srcRect = sprite.srcRect;

				// Set the dest rect with the x,y pos to be rendered
				SDL_Rect dstRect = {
					static_cast<int>(transform.position.x - (sprite.isFixed ? 0 : camera.x)),
					static_cast<int>(transform.position.y - (sprite.isFixed ? 0 : camera.y)),
					static_cast<int>(sprite.width * transform.scale.x), 
					static_cast<int>(sprite.height * transform.scale.y)
				};

				SDL_RenderCopyEx(
					renderer, 
					assetStore->GetTexture(sprite.assetId),
					&srcRect, &dstRect, 
					transform.rotation, NULL,
					sprite.flip
				);

				// Draw PNG texture
			}
		}
};
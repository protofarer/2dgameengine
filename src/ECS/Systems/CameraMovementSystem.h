#pragma once

#include "../ECS.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/TransformComponent.h"
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

class CameraMovementSystem: public System {
	public:
		CameraMovementSystem() {
			RequireComponent<CameraFollowComponent>();
			RequireComponent<TransformComponent>();
		}

		void Update(SDL_Rect& camera) {
			for (auto entity: GetSystemEntities()) {
				auto transform = entity.GetComponent<TransformComponent>();

				// if (transform.position.x + (camera.w / 2) < Game::mapWidth) {
				// 	camera.x = transform.position.x - (Game::windowWidth / 2);
				// }
				// if (transform.position.y + (camera.h / 2) < Game::mapHeight) {
				// 	camera.y = transform.position.y - (Game::windowHeight / 2);
				// }

				// camera.x = camera.x < 0 ? 0 : camera.x;
				// camera.y = camera.y < 0 ? 0 : camera.y;
				// camera.x = camera.x > camera.w ? camera.w : camera.x;
				// camera.y = camera.y > camera.h ? camera.h : camera.y;

				camera.x = transform.position.x - camera.w / 2;
				camera.y = transform.position.y - camera.h / 2;
				// camera.x = std::max(std::min(0, camera.x), std::min(std::max(0, camera.x), Game::mapWidth - camera.w));
				// camera.y = std::max(std::min(0, camera.y), std::min(std::max(0, camera.y), Game::mapHeight - camera.y));
				camera.x = glm::clamp<int>(camera.x, 0, Game::mapWidth - camera.w);
				camera.y = glm::clamp<int>(camera.y, 0, Game::mapHeight - camera.h);
			}
		}
};
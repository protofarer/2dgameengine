#pragma once

#include <glm/glm.hpp>
#include "../Components/BoxColliderComponent.h"
#include "../Components/TransformComponent.h"
#include "../EventBus/EventBus.h"
#include"../Events/CollisionEvent.h"

class CollisionSystem: public System {
	public:
		CollisionSystem() {
			RequireComponent<TransformComponent>();
			RequireComponent<BoxColliderComponent>();
		}

		void Update(std::unique_ptr<EventBus>& eventBus) {
			auto entities = GetSystemEntities();

			// iterator approach, each i is a pointer to an entity
			// don't duplicate checks
			for (auto i = entities.begin(); i != entities.end(); i++) {
				Entity a = *i;
				auto atx = a.GetComponent<TransformComponent>();
				auto acx = a.GetComponent<BoxColliderComponent>();

				int aLeft = atx.position.x + acx.offset.x;
				int aRight = atx.position.x + acx.offset.x + (acx.width * atx.scale.x);
				int aTop = atx.position.y + acx.offset.y;
				int aBottom = atx.position.y + acx.offset.y + (acx.height * atx.scale.y);

				for (auto j = i; j != entities.end(); j++) {
					Entity b = *j;

					if (a == b) continue;	// bypass if same entity

					auto btx = b.GetComponent<TransformComponent>();
					auto bcx = b.GetComponent<BoxColliderComponent>();

					int bLeft = btx.position.x + bcx.offset.x;
					int bRight = btx.position.x + bcx.offset.x + (bcx.width * btx.scale.x);
					int bTop = btx.position.y + bcx.offset.y;
					int bBottom = btx.position.y + bcx.offset.y + (bcx.height * btx.scale.y);

					bool isColliding = (
						aLeft < bRight &&
						aRight > bLeft &&
						aTop < bBottom &&
						aBottom > bTop
					);

					if(isColliding) {
						Logger::Log("Entity " + std::to_string(a.GetId()) + " is colliding with Entity " + std::to_string(b.GetId()));
						eventBus->EmitEvent<CollisionEvent>(a, b);
					}

				}
			}
		}

		void Render(SDL_Renderer* renderer, SDL_Rect& camera) {
			for (auto entity: GetSystemEntities()) {
				auto eTx = entity.GetComponent<TransformComponent>();
				auto eCx = entity.GetComponent<BoxColliderComponent>();
				SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
				SDL_Rect colliderRect = { 
					static_cast<int>(eTx.position.x + eCx.offset.x - camera.x), 
					static_cast<int>(eTx.position.y + eCx.offset.y - camera.y), 
					static_cast<int>(eCx.width * eTx.scale.x), 
					static_cast<int>(eCx.height * eTx.scale.y) 
				};

				SDL_RenderDrawRect(renderer, &colliderRect);
			}
		}
};
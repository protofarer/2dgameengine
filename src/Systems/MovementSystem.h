#pragma once

#include "../ECS/ECS.h"
#include "../Events/CollisionEvent.h"
#include "../EventBus/EventBus.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"
#include "../Components/SpriteComponent.h"

class MovementSystem: public System {
	public:
		MovementSystem() {
			RequireComponent<TransformComponent>();
			RequireComponent<RigidBodyComponent>();
		}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
			eventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::onCollision);
		}

		void onCollision(CollisionEvent& event) {
			Entity a = event.a;
			Entity b = event.b;

			if (b.BelongsToGroup("obstacles") && a.BelongsToGroup("enemies")) {
				OnEnemyHitsObstacle(a, b);
			}

			if (a.BelongsToGroup("obstacles") && b.BelongsToGroup("enemies")) {
				OnEnemyHitsObstacle(b, a);
			}
		}

		void OnEnemyHitsObstacle(Entity enemy, Entity obstacle) {
			if (enemy.HasComponent<RigidBodyComponent>() && enemy.HasComponent<SpriteComponent>()) {
				auto& rigidbody = enemy.GetComponent<RigidBodyComponent>();
				auto& sprite = enemy.GetComponent<SpriteComponent>();

				if (rigidbody.velocity.x != 0) {
					rigidbody.velocity.x *= -1;
					sprite.flip = sprite.flip == SDL_FLIP_NONE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
				}
				if (rigidbody.velocity.y != 0) {
					rigidbody.velocity.y *= -1;
					sprite.flip = sprite.flip == SDL_FLIP_NONE ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
				}
			}
		}

		void Update(double dt) {
			for (auto entity: GetSystemEntities()) {
			// Update entity sys based on velocity for every frame
				auto& transform = entity.GetComponent<TransformComponent>();
				const auto rigidbody = entity.GetComponent<RigidBodyComponent>();
				const auto sprite = entity.GetComponent<SpriteComponent>();

				transform.position.x += rigidbody.velocity.x * dt;
				transform.position.y += rigidbody.velocity.y * dt;
				if (entity.HasTag("player")) {
					transform.position.x = transform.position.x < 0 ? 0 : transform.position.x;
					transform.position.x = transform.position.x + (transform.scale.x * sprite.width) >= Game::mapWidth ? Game::mapWidth - (transform.scale.x * sprite.width) : transform.position.x;
					transform.position.y = transform.position.y < 0 ? 0 : transform.position.y;
					transform.position.y = transform.position.y + (transform.scale.y * sprite.height) >= Game::mapHeight ? Game::mapHeight - (transform.scale.y * sprite.height) : transform.position.y;
				}

				int cullMargin = 100;
				bool isEntityOutsideMap = (
					transform.position.x < 0 - cullMargin ||
					transform.position.x >= Game::mapWidth + cullMargin ||
					transform.position.y < 0 - cullMargin ||
					transform.position.y >= Game::mapHeight + cullMargin
				);
				if (isEntityOutsideMap && !entity.HasTag("player")) {
					entity.Kill();
				}

				// Logger::Log("Entity id = " + std::to_string(entity.GetId()) + " position is now (" + std::to_string(transform.position.x) + ", " + std::to_string(transform.position.y) + ")");
			}
		}
};
#pragma once

#include "../ECS/ECS.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/TransformComponent.h"

class MovementSystem: public System {
	public:
		MovementSystem() {
			RequireComponent<TransformComponent>();
			RequireComponent<RigidBodyComponent>();
		}

		void Update(double dt) {
			for (auto entity: GetSystemEntities()) {
			// Update entity sys based on velocity for every frame
				auto& transform = entity.GetComponent<TransformComponent>();
				const auto rigidbody = entity.GetComponent<RigidBodyComponent>();

				transform.position.x += rigidbody.velocity.x * dt;
				transform.position.y += rigidbody.velocity.y * dt;


				bool isEntityOutsideMap = (
					transform.position.x < 0 ||
					transform.position.y < 0 ||
					transform.position.x >= Game::mapWidth ||
					transform.position.y >= Game::mapHeight
				);
				if (isEntityOutsideMap && !entity.HasTag("player")) {
					entity.Kill();
				}

				// Logger::Log("Entity id = " + std::to_string(entity.GetId()) + " position is now (" + std::to_string(transform.position.x) + ", " + std::to_string(transform.position.y) + ")");
			}
		}
};
#pragma once

#include "../ECS/ECS.h"
#include "../EventBus/EventBus.h"
#include "../Events/KeyPressedEvent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/KeyboardControlComponent.h"
#include "../Components/TransformComponent.h"

class KeyboardControlSystem: public System {
	public:
		KeyboardControlSystem() {
			RequireComponent<KeyboardControlComponent>();
			RequireComponent<SpriteComponent>();
			RequireComponent<RigidBodyComponent>();
			RequireComponent<TransformComponent>();
		}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
			eventBus->SubscribeToEvent<KeyPressedEvent, KeyboardControlSystem>(
				this, &KeyboardControlSystem::OnKeyPressed
			);
		}

		void OnKeyPressed(KeyPressedEvent& event) {
			// std::string keyCode = std::to_string(event.symbol);
			// std::string keySymbol(1, event.symbol);
			// Logger::Log("Key pressed event emitted: [" + keyCode + "] " + keySymbol);

			for (auto entity: GetSystemEntities()) {
				const auto keyboardcontrol = entity.GetComponent<KeyboardControlComponent>();
				auto& sprite = entity.GetComponent<SpriteComponent>();
				auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
				auto transform = entity.GetComponent<TransformComponent>();

				switch (event.symbol) {
					case SDLK_UP:
						rigidbody.velocity = keyboardcontrol.upVelocity;
						sprite.srcRect.y = sprite.height * 0;
						transform.rotation = 270;
						break;
					case SDLK_RIGHT:
						rigidbody.velocity = keyboardcontrol.rightVelocity;
						sprite.srcRect.y = sprite.height * 1;
						transform.rotation = 0;
						break;
					case SDLK_DOWN:
						rigidbody.velocity = keyboardcontrol.downVelocity;
						sprite.srcRect.y = sprite.height * 2;
						transform.rotation = 90;
						break;
					case SDLK_LEFT:
						rigidbody.velocity = keyboardcontrol.leftVelocity;
						sprite.srcRect.y = sprite.height * 3;
						transform.rotation = 180;
						break;
				}
			}
		}
};
#pragma once

#include "../ECS.h"
#include "../Components/BoxColliderComponent.h"
#include "../../EventBus/EventBus.h"
#include "../../Events/CollisionEvent.h"

class DamageSystem: public System {
	public:
		DamageSystem() {
			RequireComponent<BoxColliderComponent>();
		}

		void SubscribeToEvents(std::unique_ptr<EventBus>& eventBus) {
			eventBus->SubscribeToEvent<CollisionEvent, DamageSystem>(this, &DamageSystem::onCollision);
		}

		void onCollision(CollisionEvent& event) {
			Logger::Log("The damage system received an event collision between entities " 
				+ std::to_string(event.a.GetId()) + " and " + std::to_string(event.b.GetId()));
			event.a.Kill();
			event.b.Kill();
		}

		void Update() {

		}
};
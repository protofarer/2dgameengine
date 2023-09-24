#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

// deleteme after debug segfault from createentity
#include <iostream>

size_t IComponent::nextId = 0;

size_t Entity::GetId() const { return id; }

void System::AddEntityToSystem(Entity entity) {
	entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
	entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
		return entity == other;
	}), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
	return entities;
}

const Signature& System::GetComponentSignature() const {
	return componentSignature;
}

Entity Registry::CreateEntity() {
	size_t entityId = numEntities++;

	Entity entity(entityId);
	entity.registry = this;
	entitiesToBeAdded.insert(entity);

	// make sure entityComponentSignatures vector can fit new entity
	if (entityId >= entityComponentSignatures.size()) {
		entityComponentSignatures.resize(entityId + 1);
	}

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;
}

// if entity has all components required by a system, add to system
void Registry::AddEntityToSystems(Entity entity) {
	const auto entityId = entity.GetId();

	const auto& entityComponentSignature = entityComponentSignatures[entityId];

	for (auto& system: systems) {
		const auto& systemComponentSignature = system.second->GetComponentSignature();

		bool isInterested = (
			entityComponentSignature & systemComponentSignature
		) == systemComponentSignature; // bitwise AND

		if (isInterested) {
			system.second->AddEntityToSystem(entity);
		}
	}
}

void Registry::Update() {
	// add entities that are waiting in queue
	for (auto entity: entitiesToBeAdded) {
		 AddEntityToSystems(entity);
	}
	entitiesToBeAdded.clear();
}
#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

void System::AddEntityToSystem(Entity entity) {
	entities.push_back(entity);
}

void System::RemoveEntityFromSystem(Entity entity) {
	entities.erase(std::remove_if(entities.begin(), entities.end(), [&entity](Entity other) {
		return entity.GetId() == other.GetId();
	}), entities.end());
}

std::vector<Entity> System::GetSystemEntities() const {
	return entities;
}

const Signature& System::GetComponentSignature() const {
	return componentSignature;
}

int Entity::GetId() const { return id; }

Entity Registry::CreateEntity() {
	int entityId;
	entityId = numEntities++;
	Entity entity(entityId);
	entitiesToBeAdded.insert(entity);

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr); // putting nothing there during resize, thus nullptr
	}

	if (!componentPools[componentId]) {
		Pool<TComponent>* newComponentPool = new Pool<TComponent>();
		componentPools[componentId] = newComponentPool;
	}

	// ? is type cast needed?
	Pool<TComponent>* componentPool = Pool<TComponent>(componentPools[componentId]);

	if (entityId >= componentPool->GetSize()) {
		componentPool->Resize(numEntities);
	}

	// forward args to constructor
	TComponent newComponent(std::forward<TArgs>(args)...);

	// remember ea Component is strictly data related to entity
	component->Set(entityId, newComponent);

	// update the entity's component signature for the added component
	entityComponentSignatures[entityId].set(componentId);
}

void Registry::Update() {
	// TODO add entities that are waiting in queue
	// TODO remove - " -
}
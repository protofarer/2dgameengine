#include "ECS.h"

void System::AddEntityToSystem(Entity entity) {
	entities.push_back(entity);
}

#include <algorithm>
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

int Entity::GetId() const {
	return id;
}
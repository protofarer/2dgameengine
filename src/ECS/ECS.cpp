#include "ECS.h"
#include "../Logger/Logger.h"
#include <algorithm>

size_t IComponent::nextId = 0;

size_t Entity::GetId() const { return id; }

void Entity::Kill() {
	registry->KillEntity(*this);
}

void Entity::Tag(const std::string& tag) const {
	registry->TagEntity(*this, tag);
}

bool Entity::HasTag(const std::string& tag) const {
	return registry->EntityHasTag(*this, tag);
}

void Entity::Group(const std::string& group) const {
	registry->GroupEntity(*this, group);
}

bool Entity::BelongsToGroup(const std::string& group) const {
	return registry->EntityBelongsToGroup(*this, group);
}

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
	size_t entityId;
	if (freeIds.empty()) {
		entityId = numEntities++;

		// make sure entityComponentSignatures vector can fit new entity
		if (entityId >= entityComponentSignatures.size()) {
			entityComponentSignatures.resize(entityId + 1);
		}
	} else {
		// Reuse id from list of prev removed entities
		entityId = freeIds.front();
		freeIds.pop_front();
	}

	Entity entity(entityId);
	entity.registry = this;
	entitiesToBeAdded.insert(entity);

	Logger::Log("Entity created with id = " + std::to_string(entityId));

	return entity;
}

void Registry::KillEntity(Entity entity) {
	entitiesToBeKilled.insert(entity);
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

void Registry::RemoveEntityFromSystems(Entity entity) {
	for (auto system: systems) {
		system.second->RemoveEntityFromSystem(entity);
	}
}

// Tag Management
void Registry::TagEntity(Entity entity, const std::string& tag) {
	entityPerTag.emplace(tag, entity);
	tagPerEntity.emplace(entity.GetId(), tag);
}

bool Registry::EntityHasTag(Entity entity, const std::string& tag) const {
	// fast check if id even exists
	if (tagPerEntity.find(entity.GetId()) == tagPerEntity.end()) {
		return false;
	}
	// now check if current entity is same as sought entity
	return entityPerTag.find(tag)->second == entity;
}

Entity Registry::GetEntityByTag(const std::string& tag) const {
	return entityPerTag.at(tag);
}

void Registry::RemoveEntityTag(Entity entity) {
	auto taggedEntity = tagPerEntity.find(entity.GetId());
	if (taggedEntity != tagPerEntity.end()) {
		auto tag = taggedEntity->second;
		entityPerTag.erase(tag);
		tagPerEntity.erase(taggedEntity);
	}
}

// Group Management
void Registry::GroupEntity(Entity entity, const std::string& group) {
	entitiesPerGroup.emplace(group, std::set<Entity>());		// only inserts if key(group) not already present
	entitiesPerGroup[group].emplace(entity);
	groupPerEntity.emplace(entity.GetId(), group);
}

bool Registry::EntityBelongsToGroup(Entity entity, const std::string& group) const {
	if (entitiesPerGroup.find(group) == entitiesPerGroup.end()) {
		return false;
	}
	auto groupEntities = entitiesPerGroup.at(group); // ! this panicks when group not yet created, eg no projectile emitted yet and this function is called to check "projectiles" group in DamageSystem :: fixed, see above line
	return groupEntities.find(entity.GetId()) != groupEntities.end();
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string& group) const {
	auto& setOfEntities =  entitiesPerGroup.at(group);
	return std::vector<Entity>(setOfEntities.begin(), setOfEntities.end());
}

void Registry::RemoveEntityGroup(Entity entity) {
	auto groupedEntity = groupPerEntity.find(entity.GetId());
	if (groupedEntity != groupPerEntity.end()) {
		auto group = entitiesPerGroup.find(groupedEntity->second);
		if (group != entitiesPerGroup.end()) {
			auto entityInGroup = group->second.find(entity);
			if (entityInGroup != group->second.end()) {
				group->second.erase(entityInGroup);
			}
		}
		groupPerEntity.erase(groupedEntity);
	}
}

void Registry::Update() {
	// add entities that were waiting in queue
	for (auto entity: entitiesToBeAdded) {
		 AddEntityToSystems(entity);
	}
	entitiesToBeAdded.clear();

	for (auto entity: entitiesToBeKilled) {
		 RemoveEntityFromSystems(entity);
		 entityComponentSignatures[entity.GetId()].reset();

		// Remove entity from component pools
		for (auto pool: componentPools) {
			// only remove entity if pool is not null
			if (pool) {
				pool->RemoveEntityFromPool(entity.GetId());
			}
		}

		 // Free entity id for reuse
		 freeIds.push_back(entity.GetId());

		 // Remove any traces of entity from tag and groups
		 RemoveEntityGroup(entity);
		 RemoveEntityTag(entity);
	}
	entitiesToBeKilled.clear();
	entitiesToBeAdded.clear();
}

#pragma once

#include "../Logger/Logger.h"
#include <vector>
#include <bitset>
#include <set>
#include <deque>
#include <unordered_map>
#include <typeindex>
#include <memory>

// debug
#include <iostream>

const unsigned int MAX_COMPONENTS = 32;

////////////////////////////////////////////////////////////////////////////////
// Signature
////////////////////////////////////////////////////////////////////////////////
// We use a bitset (1s and 0s) to keep track of which components an entity has,
// and also helps keep track of which entities a system is interested in.
// Define which components an entity has
// Enables systems to find relevant entities
////////////////////////////////////////////////////////////////////////////////
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
	protected:
		static size_t nextId;
};

template <typename T>
class Component: public IComponent {
	public:
		static size_t GetId() {
			static auto id = nextId++;
			return id;
		}
};

class Entity {
	private:
		size_t id;
	public:
		Entity(size_t id): id(id) {};
		Entity(const Entity& entity) = default;
		size_t GetId() const; // declare doesnt modify members/internals of class
		void Kill();

		Entity& operator =(const Entity& other) = default;
		bool operator ==(const Entity& other) const { return id == other.id; };
		bool operator !=(const Entity& other) const { return id != other.id; };
		bool operator >(const Entity& other) const { return id > other.id; };
		bool operator <(const Entity& other) const { return id < other.id; };

		template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
		template <typename TComponent> void RemoveComponent();
		template <typename TComponent> bool HasComponent() const;
		template <typename TComponent> TComponent& GetComponent() const;

		// Tag and group management via entity
		void Tag(const std::string& tag) const;
		bool HasTag(const std::string& tag) const;
		void Group(const std::string& group) const;
		bool BelongsToGroup(const std::string& group) const;

		class Registry* registry; // forward declaration so Entity has pointer access to Registry
};

////////////////////////////////////////////////////////////////////////////////
// System
////////////////////////////////////////////////////////////////////////////////
// The system processes entities that contain a specific signature
////////////////////////////////////////////////////////////////////////////////
class System {
	private:
		Signature componentSignature;
		std::vector<Entity> entities;

	public:
		System() = default;
		~System() = default;

		void AddEntityToSystem(Entity entity);
		void RemoveEntityFromSystem(Entity entity);
		std::vector<Entity> GetSystemEntities() const;
		const Signature& GetComponentSignature() const;
		template <typename TComponent> void RequireComponent();
};

////////////////////////////////////////////////////////////////////////////////
// Pool
////////////////////////////////////////////////////////////////////////////////
// A pool is just a vector (contiguous data) of objects of type T
// * so that Registry doesnt have to specify type of Pool (wrap it in an abstract class without a type)
///////////////////////////////////////////////////////////////
class IPool {
	public:
		virtual ~IPool() = default; // forces class to be abstract
		virtual void RemoveEntityFromPool(int entityId) = 0; // pure virtual method, must override impl in Pool
};

template <typename T>
class Pool: public IPool {
	private:
		// We keep track of the vector of objects and current number of elements
		std::vector<T> data;
		int size;

		// Helper maps track entity ids fper index, ensuring packed data
		std::unordered_map<int, int> entityIdToIndex;
		std::unordered_map<int, int> indexToEntityId;

	public:
		Pool(int capacity = 100) { 
			size = 0;
			data.resize(capacity); 
		}
		virtual ~Pool() = default;

		bool IsEmpty() const { 
			return size == 0; 
		}

		int GetSize() const { 
			return size; 
		}

		void Resize(int n) { 
			data.resize(n); 
		}

		void Clear() { 
			data.clear(); 
			size = 0; 
		}

		void Add(T object) { 
			data.push_back(object); 
		}

		void Set(int entityId, T object) { 
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
				// If entity already exists, replace component obj
				int index = entityIdToIndex[entityId];
				data[index] = object;
			} else {
				// when add new object, keep track of entity ids and their vector index
				int index = size;
				entityIdToIndex.emplace(entityId, index);
				indexToEntityId.emplace(index, entityId);
				if (index >= data.capacity()) {
					// double capacity when needed
					data.resize(size * 2);
				}
				data[index] = object;
				size++;
			}
		}

		void Remove(int entityId) {
			// remove from entityIdToIndex by simple emplace, if it exists
			int indexOfRemoved = entityIdToIndex[entityId];
			int indexOfLast = size - 1;
			data[indexOfRemoved] = data[indexOfLast];	// efficiently swap in last element into removed entity's position, thus achieving packing

			// account for change in tracker hashmaps (entityId <-> data index)
			int entityIdOfLastElement = indexToEntityId[indexOfLast];
			entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
			indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

			entityIdToIndex.erase(entityId);
			indexToEntityId.erase(indexOfLast);

			size--;
		}

		void RemoveEntityFromPool(int entityId) override {
			if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
				Remove(entityId);
			}
		}

		T& Get(int entityId) { 
			int index = entityIdToIndex[entityId];
			return static_cast<T&>(data[index]);
		}

		T& operator [](unsigned int index) const { 
			return data[index]; 
		}
};

///////////////////////////////////////////////////////////////
// REGISTRY
// Manages creatiuon and destruction of entities, add systems and components
///////////////////////////////////////////////////////////////
class Registry {
	private:
		size_t numEntities = 0;

		// Ea pool contains all the data for a certain component type
		// [Vector index = component type id]
		// [Pool index = entity id]
		std::vector<std::shared_ptr<IPool>> componentPools;

		// Vector of component sigs per entity, saying which comp is turned on for ea entity
		// [Vector index = entity id]
		std::vector<Signature> entityComponentSignatures;

		// Map of active systems
		// [Map key = system type id]
		std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

		// Entites flagged for add/remove till next registry update
		std::set<Entity> entitiesToBeAdded;
		std::set<Entity> entitiesToBeKilled;

		// Entity tags (1 tag name per entity)
		std::unordered_map<std::string, Entity> entityPerTag;
		std::unordered_map<int, std::string> tagPerEntity;

		// Entity groups (a set of entities per group name)
		std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
		std::unordered_map<int, std::string> groupPerEntity;

		// List of free entity ids that were previously removed
		std::deque<int> freeIds;

	public:
		Registry() {
			Logger::Log("Registry constructor called.");
		}

		~Registry() {
			Logger::Log("Registry destructor called.");
		}

		void Update();

		// Entity management
		Entity CreateEntity();
		void KillEntity(Entity entity);

		// Component management
		template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
		template <typename TComponent> void RemoveComponent(Entity entity);
		template <typename TComponent> bool HasComponent(Entity entity) const;
		template <typename TComponent> TComponent& GetComponent(Entity entity) const;

		// System Management
		template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
		template <typename TSystem> void RemoveSystem();
		template <typename TSystem> bool HasSystem() const;
		template <typename TSystem> TSystem& GetSystem() const;

		// Add and remove entities from their systems
		void AddEntityToSystems(Entity entity);
		void RemoveEntityFromSystems(Entity entity);

		// Tag Management
		void TagEntity(Entity entity, const std::string& tag);
		bool EntityHasTag(Entity entity, const std::string& tag) const;
		Entity GetEntityByTag(const std::string& tag) const;
		void RemoveEntityTag(Entity entity);

		// Group Management
		void GroupEntity(Entity entity, const std::string& group);
		bool EntityBelongsToGroup(Entity entity, const std::string& group) const;
		std::vector<Entity> GetEntitiesByGroup(const std::string& group) const;
		void RemoveEntityGroup(Entity entity);
};

template <typename TComponent> void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
};

template <typename TSystem, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	systems.insert(
		std::make_pair(
			std::type_index(typeid(TSystem)), 
		newSystem)
	);
}

template <typename TSystem>
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(TSystem))); // returns point to system
	systems.erase(system);
}

template <typename TSystem>
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end(); // test for whether key found
}

template <typename TSystem>
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem))); // get iterator pointer
	return *(std::static_pointer_cast<TSystem>(system->second)); // deref to value that's pointed at

}

template <typename TComponent, typename ...TArgs> 
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();

	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr); // putting nothing there during resize, thus nullptr
	}

	if (!componentPools[componentId]) {
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentId] = newComponentPool;
	}

	// ? is type cast really needed?
	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

	// superceded by the new tracker hashmaps and updated Pool::Set call below
	// if (entityId >= componentPool->GetSize()) {
	// 	componentPool->Resize(numEntities);
	// }

	// forward args to constructor
	TComponent newComponent(std::forward<TArgs>(args)...);

	componentPool->Set(entityId, newComponent);

	// remember ea Component is strictly data related to entity
	componentPool->Set(entityId, newComponent);

	// update the entity's component signature for the added component
	entityComponentSignatures[entityId].set(componentId);

	// Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " +std::to_string(entityId));
};

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();


	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	componentPool->Remove(entityId);

	// superceded by tracker hashmap and new Pool::Remove, below
	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id = " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
};

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return entityComponentSignatures[entityId].test(componentId);
};

template <typename TComponent> TComponent& Registry::GetComponent(Entity entity) const {
	const auto entityId = entity.GetId();
	const auto componentId = Component<TComponent>::GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	return componentPool->Get(entityId);
}

template <typename TComponent, typename ...TArgs> 
void Entity::AddComponent(TArgs&& ...args) {
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
};

template <typename TComponent> 
void Entity::RemoveComponent() {
	registry->RemoveComponent<TComponent>(*this);
};

template <typename TComponent> 
bool Entity::HasComponent() const {
	return registry->HasComponent<TComponent>(*this);
};

template <typename TComponent> 
TComponent& Entity::GetComponent() const {
	return registry->GetComponent<TComponent>(*this);
};
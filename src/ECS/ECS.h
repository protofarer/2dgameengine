#pragma once

#include "../Logger/Logger.h"
#include <vector>
#include <bitset>
#include <set>
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

		Entity& operator =(const Entity& other) = default;
		bool operator ==(const Entity& other) const { return id == other.id; };
		bool operator !=(const Entity& other) const { return id != other.id; };
		bool operator >(const Entity& other) const { return id > other.id; };
		bool operator <(const Entity& other) const { return id < other.id; };
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
		virtual ~IPool() {} // forces class to be abstract
};

template <typename T>
class Pool: public IPool {
	private:
		std::vector<T> data;

	public:
		Pool(int size = 100) { data.resize(size); }
		virtual ~Pool() = default;

		bool isEmpty() const { return data.empty(); }
		int GetSize() const { return data.size(); }
		void Resize(int n) { data.resize(n); }
		void Add(T object) { data.push_back(object); }
		void Set(int index, T object) { data[index] = object; }
		T& Get(int index) const { return static_cast<T>(data[index]); }
		T& operator [](unsigned int index) const { return data[index]; }
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

	public:
		Registry() {
			Logger::Log("registry constructor called");
		}

		~Registry() {
			Logger::Log("registry destructor called");
		}

		void Update();

		// Entity management
		Entity CreateEntity();

		// Component management
		template <typename TComponent, typename ...TArgs> 
		void AddComponent(Entity entity, TArgs&& ...args);

		template <typename TComponent>
		void RemoveComponent(Entity entity);

		template <typename TComponent>
		bool HasComponent(Entity entity) const;

		// System Management
		template <typename TSystem, typename ...TArgs>
		void AddSystem(TArgs&& ...args);

		template <typename TSystem>
		void RemoveSystem();

		template <typename TSystem>
		bool HasSystem() const;

		template <typename TSystem>
		TSystem& GetSystem() const;

		// checks the component sig of entity, add entity to systems interested in it
		void AddEntityToSystems(Entity entity);
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

	if (entityId >= componentPool->GetSize()) {
		componentPool->Resize(numEntities);
	}
	

	// forward args to constructor
	TComponent newComponent(std::forward<TArgs>(args)...);

	// remember ea Component is strictly data related to entity
	componentPool->Set(entityId, newComponent);

	// update the entity's component signature for the added component
	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id = " + std::to_string(componentId) + " was added to entity id " +std::to_string(entityId));
};

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	entityComponentSignatures[entityId].set(componentId, false);
};

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
	const auto componentId = Component<TComponent>::GetId();
	const auto entityId = entity.GetId();
	return entityComponentSignatures[entityId].test(componentId);
};
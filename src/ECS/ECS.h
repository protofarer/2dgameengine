#pragma once

#include <bitset>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>

///////////////////////////////////////////////////////////////
// ENTITY
///////////////////////////////////////////////////////////////
class Entity {
	private:
		int id;
	public:
		Entity(int id): id(id) {};
		Entity(Entity& entity) = default;
		int GetId() const; // declare doesnt modify members/internals of class

		Entity& operator =(const Entity& other) = default;
		bool operator ==(const Entity& other) const { return id == other.id; };
		bool operator !=(const Entity& other) const { return id != other.id; };
		bool operator >(const Entity& other) const { return id > other.id; };
		bool operator <(const Entity& other) const { return id < other.id; };
};

///////////////////////////////////////////////////////////////
// POOL
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
		vod Add(T object) { data.push_back(object); }
		void Set(int index, T object) { data[index] = obect; }
		T& Get(int index) const { return static_cast<T>(data[index]); }
		T& operator [](unsigned int index) const { return data[index]; }
};

///////////////////////////////////////////////////////////////
// REGISTRY
// Manages creatiuon and destruction of entities, add systems and components
///////////////////////////////////////////////////////////////
class Registry {
	public:
		Registry() = default;

		void Update();

		Entity CreateEntity();

		void KillEntity();

		template <typename TComponent, typename ...TArgs> 
		void AddComponent(Entity entity, TArgs&& ...args);

		void AddEntityToSystem(Entity entity);

		// RemoveComponent(Entity entity)
		// HasComponent(Entity entity)
		// GetComponent(Entity entity)
		//
		// AddSystem()
		// RemoveSystem()
		// HasSystem()
		// GetSystem()
	private:
		int numEntities = 0;
		std::set<Entity> entitiesToBeAdded; // await till next registry update
		std::set<Entity> entitiesToBeKilled; // await till next registry update

		// Ea pool contains all the data for a certain component type
		// [Vector index = component type id]
		// [Pool index = entity id]
		std::vector<IPool*> componentPools;

		// Vector of component sigs per entity, saying which comp is turned on for ea entity
		// [Vector index = entity id]
		std::vector<Signature> entityComponentSignatures;

		std::unordered_map<std::type_index, System*> systems;
};

const unsigned int MAX_COMPONENTS = 32;

// Define which components an entity has
// Tracks which entities a system is interested in
typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
	protected:
		static int nextId;
};

template <typename T>
class Component: public IComponent {
	static int GetId() {
		static auto id = nextId++;
		return id;
	}
};



// System processes entities that contain a specific signature
// Signature is a bitset that says what components are active for a system
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

template <typename TComponent> void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}
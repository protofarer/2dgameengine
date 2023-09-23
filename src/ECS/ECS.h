#pragma once

#include <bitset>
#include <vector>

class Entity {
	private:
		int id;
	public:
		Entity(int id): id(id) {};
		int GetId() const; // declare doesnt modify members/internals of class

		bool operator ==(const Entity& other) const { return id == other.id; };
		bool operator !=(const Entity& other) const { return id != other.id; };
		bool operator >(const Entity& other) const { return id > other.id; };
		bool operator <(const Entity& other) const { return id < other.id; };
};

class Registry {

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
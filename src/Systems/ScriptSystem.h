#pragma once

#include "../ECS/ECS.h"
#include "../Components/ScriptComponent.h"
#include "../Components/TransformComponent.h"

// Declare native C++ functions to bind with Lua functions
std::tuple<double, double> GetEntityPosition(Entity entity) {
	if (entity.HasComponent<TransformComponent>()) {
		const auto transform = entity.GetComponent<TransformComponent>();
		return std::make_tuple(transform.position.x, transform.position.y);
	} else {
		Logger::Err("Attempted access to entity without a TransformComponent");
		return std::make_tuple(0.0, 0.0);
	}
}

void SetEntityPosition(Entity entity, double x, double y) {
	if (entity.HasComponent<TransformComponent>()) {
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.position.x = x;
		transform.position.y = y;
	} else {
		Logger::Err("Cannot set position of entity without TransformComponent");
	}
}

std::tuple<double, double> GetEntityVelocity(Entity entity) {
	if (entity.HasComponent<RigidBodyComponent>()) {
		const auto rigidbody = entity.GetComponent<RigidBodyComponent>();
		return std::make_tuple(rigidbody.velocity.x, rigidbody.velocity.y);
	} else {
		Logger::Err("Attempted access to an entity without a RigidBodyComponent");
		return std::make_tuple(0.0, 0.0);
	}
}

void SetEntityVelocity(Entity entity, double x, double y) {
	if (entity.HasComponent<RigidBodyComponent>()) {
		auto& rigidbody = entity.GetComponent<RigidBodyComponent>();
		rigidbody.velocity.x = x;
		rigidbody.velocity.y = y;
	} else {
		Logger::Err("Cannot set velocity of an entity without RigidBodyComponent");
	}
}

void SetEntityRotation(Entity entity, double rotation) {
	if (entity.HasComponent<TransformComponent>()) {
		auto& transform = entity.GetComponent<TransformComponent>();
		transform.rotation = rotation;
	} else {
		Logger::Err("Cannot set rotation of an entity without TransformComponent");
	}
}

void SetEntityProjectileVelocity(Entity entity, double projectileVelocityX, double projectileVelocityY) {
	if (entity.HasComponent<ProjectileEmitterComponent>()) {
		auto& projectileEmitter = entity.GetComponent<ProjectileEmitterComponent>();
		projectileEmitter.projectileVelocity.x = projectileVelocityX;
		projectileEmitter.projectileVelocity.y = projectileVelocityY;
	} else {
		Logger::Err("Cannot set projectile velocity of an entity without ProjectileEmitterComponent");
	}
}

void SetEntityAnimationFrame(Entity entity, int currentFrame) {
	if (entity.HasComponent<AnimationComponent>()) {
		auto& animation = entity.GetComponent<AnimationComponent>();
		animation.currentFrame = currentFrame;
	} else {
		Logger::Err("Cannot set animation frame of an entity without AnimationComponent");
	}
}

class ScriptSystem: public System {
	public:
		ScriptSystem() {
			RequireComponent<ScriptComponent>();
		}

		void CreateLuaBindings(sol::state& lua) {
			// Create "entity" usertype for Lua
			lua.new_usertype<Entity>(
				"entity", 
				"get_id", &Entity::GetId,
				"destroy", &Entity::Kill,
				"has_tag", &Entity::HasTag,
				"belongs_to_group", &Entity::BelongsToGroup
				);

			// Create bindings from C++ to Lua functions
			lua.set_function("get_position", GetEntityPosition);
			lua.set_function("get_velocity", GetEntityVelocity);
			lua.set_function("set_position", SetEntityPosition);
			lua.set_function("set_velocity", SetEntityVelocity);
			lua.set_function("set_rotation", SetEntityRotation);
			lua.set_function("set_projectile_velocity", SetEntityProjectileVelocity);
			lua.set_function("set_animation_frame", SetEntityAnimationFrame);
		}

		void Update(double dt, int elapsedTime) {
			for (auto entity: GetSystemEntities()) {
				const auto script = entity.GetComponent<ScriptComponent>();
				script.func(entity, dt, elapsedTime); // invoke the sol::function
			}
		}
};
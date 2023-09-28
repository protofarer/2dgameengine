#pragma once

#include <SDL2/SDL.h>
#include <glm/glm.hpp>

struct ProjectileEmitterComponent {
	glm::vec2 projectileVelocity;
	int repeatFrequency;
	int projectileDuration;
	int hitPercentDamage;
	bool isFriendly;
	int lastEmissionTime;	// time of last projectile emit

	ProjectileEmitterComponent(
		glm::vec2 projectileVelocity = glm::vec2(0), 
		int repeatFrequency = 0,
		int projectileDuration = 10000,
		int hitPercentDamage = 10,
		bool isFriendly = false
	): 
		projectileVelocity(projectileVelocity), 
		repeatFrequency(repeatFrequency), 
		projectileDuration(projectileDuration), 
		hitPercentDamage(hitPercentDamage),
		isFriendly(isFriendly),
		lastEmissionTime(SDL_GetTicks())
	{}
};
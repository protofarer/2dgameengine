#pragma once

#include "../ECS/ECS.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"

class AnimationSystem: public System {
	public:
		AnimationSystem() {
			RequireComponent<SpriteComponent>();
			RequireComponent<AnimationComponent>();
		}

		void Update() {
			for (auto entity: GetSystemEntities()) {
				auto& animation = entity.GetComponent<AnimationComponent>();
				auto& sprite = entity.GetComponent<SpriteComponent>();
				// numFrames = total frames avail in spritesheet
				// currentFrame = any between [0, numFrames)
				// framespeed = frames/second

				// alternate between 0 and 1 every 1/5 sec
				animation.currentFrame = ((SDL_GetTicks() - animation.startTime) * animation.frameSpeedRate / 1000) % animation.numFrames;
				sprite.srcRect.x = animation.currentFrame * sprite.width;
			}
		}
};
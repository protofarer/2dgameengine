#pragma once

#include "../ECS.h"
#include <string>
#include <SDL2/SDL.h>

struct SpriteComponent {
	std::string assetId;
	int width;
	int height;
	int zIndex;
	SDL_Rect srcRect;

	SpriteComponent(
		std::string assetId = "", 
		int zIndex = 0,
		int width = 0, 
		int height = 0, 
		int srcRectX = 0, 
		int srcRectY = 0
	): 
		assetId(assetId), 
		zIndex(zIndex),
		width(width), 
		height(height)
	{
		this->srcRect = { srcRectX, srcRectY, width, height };
	};
};
		// this->assetId = assetId;
		// this->width = width;
		// this->height = height;
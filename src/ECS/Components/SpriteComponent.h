#pragma once

#include "../ECS.h"

struct SpriteComponent {
	int width;
	int height;

	SpriteComponent(int width = 0, int height = 0): width(width), height(height) {};
};
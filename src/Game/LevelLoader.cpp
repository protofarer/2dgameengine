#include "Game.h"
#include "LevelLoader.h"
#include "../Components/TransformComponent.h"
#include "../Components/RigidBodyComponent.h"
#include "../Components/SpriteComponent.h"
#include "../Components/AnimationComponent.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/CameraFollowComponent.h"
#include "../Components/KeyboardControlComponent.h"
#include "../Components/ProjectileEmitterComponent.h"
#include "../Components/HealthComponent.h"
#include "../Components/TextLabelComponent.h"
#include <fstream>
#include <sstream>
#include <sol/sol.hpp>

LevelLoader::LevelLoader() {

}
LevelLoader::~LevelLoader() {

}

void LevelLoader::LoadLevel(
	sol::state& lua,
	const std::unique_ptr<Registry>& registry, 
	const std::unique_ptr<AssetStore>& assetStore, 
	SDL_Renderer* renderer,int levelNumber
) {
 	// load without executing, validate script
	sol::load_result script = lua.load_file("./assets/scripts/Level" + std::to_string(levelNumber) + ".lua");
	if (!script.valid()) {
		sol::error err = script;
		std::string errorMessage = err.what();
		Logger::Err("Error loading the lua script: " + errorMessage);
		return;
	}

	// Execute script
	lua.script_file("./assets/scripts/Level" + std::to_string(levelNumber) + ".lua");

	sol::table level = lua["Level"];

	/////////////////////////////////////////////////////////////////////////////
	// Read level assets
	/////////////////////////////////////////////////////////////////////////////
	sol::table assets = level["assets"];

	int i = 0;
	while (true) {
		sol::optional<sol::table> hasAsset = assets[i];
		if (hasAsset == sol::nullopt) {
			break;
		}

		sol::table asset = assets[i];
		std::string assetType = asset["type"];
		if (assetType == "texture") {
			assetStore->AddTexture(renderer, asset["id"], asset["file"]);
		} else if (assetType == "font") {
			assetStore->AddFont(asset["id"], asset["file"], asset["font_size"]);
		}

		i++;
	}


	/////////////////////////////////////////////////////////////////////////////
	// Read level tilemap information
	/////////////////////////////////////////////////////////////////////////////
	sol::table map = level["tilemap"];
	std::string mapFilePath = map["map_file"];
	std::string mapTextureAssetId = map["texture_asset_id"];
	int mapNumRows = map["num_rows"];
	int mapNumCols = map["num_cols"];
	int tileSize = map["tile_size"];
	double mapScale = map["scale"];

	// 1. read map -> 2d array of srcImage indices
	std::vector<std::vector<int>> mapMatrix(mapNumRows, std::vector<int>(mapNumCols, 0));

	std::ifstream file(mapFilePath);
	if (!file) {
		std::cerr << "Error: Could not open file." << std::endl;
		// return matrix; // Return an empty matrix on error
	}

	std::string line;
	int row = 0;

	while (std::getline(file, line) && row < mapNumRows) {
		std::istringstream iss(line);
		int col = 0;
		std::string token;

		while (std::getline(iss, token, ',')) {
				if (col < mapNumCols) {
					try {
						mapMatrix[row][col] = std::stoi(token); // Convert the token to an integer
					} catch (const std::invalid_argument& e) {
						// Handle invalid integers if needed
						std::cerr << "Invalid integer: " << token << std::endl;
					}
				}
				col++;
		}
		row++;
	}
	file.close();
	/////////////////////////////////////////////////////////////////////////////
	// Initialize level tiles
	/////////////////////////////////////////////////////////////////////////////
	// 2. create tilemap entities, locate them to game world coordinates
	// LevelLoader::CreateTileMapEntities(matrix, registry);

	// 2. iterate through map, each (col, row) index corresponds to some column width = windowWidth / maxCols, row height = windowHeight / maxRows
	// 2.a create entity -> specify transform, specify sprite (set srcRect x y)
	for (int y = 0; y < mapNumRows; y++) {
		for (int x = 0; x < mapNumCols; x++) {
			int img_no = mapMatrix[y][x];
			int srcRow = img_no / 10;
			int srcCol = img_no % 10;
			int srcX = tileSize * srcCol;
			int srcY = tileSize * srcRow;
			int posX = tileSize * mapScale * x;
			int posY = tileSize * mapScale * y;
			Entity tile = registry->CreateEntity();
			tile.Group("tiles");
			tile.AddComponent<TransformComponent>(glm::vec2(posX, posY), glm::vec2(mapScale, mapScale), 0.0);
			tile.AddComponent<SpriteComponent>(mapTextureAssetId, tileSize, tileSize, 0,false, srcX, srcY);
		}
	}

	Game::mapWidth = mapNumCols * tileSize * mapScale;
	Game::mapHeight = mapNumRows * tileSize * mapScale;

	sol::table entities = level["entities"];
	i = 0;
	while (true) {
		sol::optional<sol::table> hasEntity = entities[i];
		if (hasEntity == sol::nullopt) {
			break;
		}

		sol::table entity = entities[i];

		Entity newEntity = registry->CreateEntity();

		sol::optional<std::string> tag = entity["tag"];
		if (tag != sol::nullopt) {
			newEntity.Tag(entity["tag"]);
		}

		sol::optional<std::string> group = entity["group"];
		if (group != sol::nullopt) {
			newEntity.Group(entity["group"]);
		}

		sol::optional<sol::table> hasComponents = entity["components"];
		if (hasComponents != sol::nullopt) {
			// Transform
			sol::optional<sol::table> transform = entity["components"]["transform"];
			if (transform != sol::nullopt) {
					newEntity.AddComponent<TransformComponent>(
						glm::vec2(
							entity["components"]["transform"]["position"]["x"],
							entity["components"]["transform"]["position"]["y"]
						),
						glm::vec2(
							entity["components"]["transform"]["scale"]["x"].get_or(1.0),
							entity["components"]["transform"]["scale"]["y"].get_or(1.0)
						),
						entity["components"]["transform"]["rotation"].get_or(0.0)
					);
			}

			// RigidBody
			sol::optional<sol::table> rigidbody = entity["components"]["rigidbody"];
			if (rigidbody != sol::nullopt) {
					newEntity.AddComponent<RigidBodyComponent>(
						glm::vec2(
							entity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
							entity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
						)
					);
			}

			// Sprite
			sol::optional<sol::table> sprite = entity["components"]["sprite"];
			if (sprite != sol::nullopt) {
					newEntity.AddComponent<SpriteComponent>(
						entity["components"]["sprite"]["texture_asset_id"],
						entity["components"]["sprite"]["width"],
						entity["components"]["sprite"]["height"],
						entity["components"]["sprite"]["z_index"].get_or(1),
						entity["components"]["sprite"]["fixed"].get_or(false),
						entity["components"]["sprite"]["src_rect_x"].get_or(0),
						entity["components"]["sprite"]["src_rect_y"].get_or(0)
					);
			}

			// Animation
			sol::optional<sol::table> animation = entity["components"]["animation"];
			if (animation != sol::nullopt) {
					newEntity.AddComponent<AnimationComponent>(
						entity["components"]["animation"]["num_frames"].get_or(1),
						entity["components"]["animation"]["speed_rate"].get_or(1)
					);
			}

			// BoxCollider
			sol::optional<sol::table> collider = entity["components"]["boxcollider"];
			if (collider != sol::nullopt) {
					newEntity.AddComponent<BoxColliderComponent>(
						entity["components"]["boxcollider"]["width"],
						entity["components"]["boxcollider"]["height"],
						glm::vec2(
							entity["components"]["boxcollider"]["offset"]["x"].get_or(0),
							entity["components"]["boxcollider"]["offset"]["y"].get_or(0)
						)
					);
			}
			
			// Health
			sol::optional<sol::table> health = entity["components"]["health"];
			if (health != sol::nullopt) {
					newEntity.AddComponent<HealthComponent>(
						static_cast<int>(entity["components"]["health"]["health_percentage"].get_or(100))
					);
			}
			
			// ProjectileEmitter
			sol::optional<sol::table> projectileEmitter = entity["components"]["projectile_emitter"];
			if (projectileEmitter != sol::nullopt) {
					newEntity.AddComponent<ProjectileEmitterComponent>(
						glm::vec2(
							entity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
							entity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
						),
						static_cast<int>(entity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
						static_cast<int>(entity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
						static_cast<int>(entity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
						entity["components"]["projectile_emitter"]["friendly"].get_or(false)
					);
			}

			// CameraFollow
			sol::optional<sol::table> cameraFollow = entity["components"]["camera_follow"];
			if (cameraFollow != sol::nullopt) {
					newEntity.AddComponent<CameraFollowComponent>();
			}

			// KeyboardControlled
			sol::optional<sol::table> keyboardControlled = entity["components"]["keyboard_controller"];
			if (keyboardControlled != sol::nullopt) {
					newEntity.AddComponent<KeyboardControlComponent>(
						glm::vec2(
							entity["components"]["keyboard_controller"]["up_velocity"]["x"],
							entity["components"]["keyboard_controller"]["up_velocity"]["y"]
						),
						glm::vec2(
							entity["components"]["keyboard_controller"]["right_velocity"]["x"],
							entity["components"]["keyboard_controller"]["right_velocity"]["y"]
						),
						glm::vec2(
							entity["components"]["keyboard_controller"]["down_velocity"]["x"],
							entity["components"]["keyboard_controller"]["down_velocity"]["y"]
						),
						glm::vec2(
							entity["components"]["keyboard_controller"]["left_velocity"]["x"],
							entity["components"]["keyboard_controller"]["left_velocity"]["y"]
						)
					);
			}
		}
		i++;
	}
}
#pragma once

#include "../ECS/ECS.h"
#include <imgui/imgui.h>
#include <imgui/imgui_sdl.h>

class RenderGUISystem: public System {
	public:
		RenderGUISystem() = default;

		void Update(const std::unique_ptr<Registry>& registry) {
			ImGui::NewFrame();

			// ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
			if (ImGui::Begin("Spawn Enemies")) {
				static int enemyXPos = 0;
				static int enemyYPos = 0;
				ImGui::InputInt("x position", &enemyXPos, 25, 100);
				ImGui::InputInt("y position", &enemyYPos, 25, 100);

				// TODO more inputs: velocity, scale, rotation, dropdown sprite texture id, angle/speed/duration/repeat projectiles, initial health

				if (ImGui::Button("Spawn tank")) {
					Entity tank = registry->CreateEntity();
					tank.Group("enemies");
					tank.AddComponent<TransformComponent>(glm::vec2(enemyXPos, enemyYPos), glm::vec2(2.0, 2.0), 0.0);
					tank.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
					tank.AddComponent<SpriteComponent>("tank-image", 32, 32, 1);
					tank.AddComponent<BoxColliderComponent>(32, 32);
					tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(100.0, 0.0), 5000, 3000, 50, false);
					tank.AddComponent<HealthComponent>(100);

				}
				ImGui::End();
			}
			ImGui::Render();
			ImGuiSDL::Render(ImGui::GetDrawData());
		}
};
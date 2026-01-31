#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Config.h"
#include "../core/Layers/ImGuiLayer.h"
#include "layers/EditorLayer.h"

// Needed to spawn the initial entity
#include "../scene/World.h"
#include "../ecs/Components.h"
#include "../scene/Scene.h"
#include "../ecs/Entity.h"

int main()
{
    // Use the Macro, not raw Log::Write (for consistency)
    // But since Macros are in the Engine lib, we might need to initialize logging first?
    // Actually, Engine constructor inits logging, so raw access is safer here.
    aether::Log::Write(aether::LogLevel::Info, "Aether Editor starting up...");

    // 1. Identity
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    // 2. Preferences
    aether::WindowSettings settings;
    settings.Width = 1280;
    settings.Height = 720;
    settings.Title = "Aether Editor";
    settings.Mode = aether::WindowMode::Maximized;
    aether::Config::Load("editor.ini", settings);

    // 3. Start Engine
    aether::Engine engine(spec, settings);

    // --- 4. INITIALIZE WORLD STATE ---
    // The Editor needs a "Sandbox" world to edit.
    auto world = std::make_unique<aether::World>("Editor Sandbox");

    // Spawn a Test Entity so we can see the Red Box
    aether::Scene* scene = world->GetScene();
    if (scene) {
        auto player = scene->CreateEntity("Player One");
        player.AddComponent<aether::SpriteComponent>({ 1.0f, 0.2f, 0.2f, 1.0f }); // Red Color

        // Log it (using Engine macro now that Engine exists)
        AETHER_CORE_INFO("Spawned Initial Entity: {0}", (uint32_t)player.GetID());
    }

    // Set the world active
    engine.SetWorld(std::move(world));
    // ---------------------------------

    // 5. Push Layers
    // Overlay: ImGui Backend
    engine.PushOverlay(new aether::ImGuiLayer());

    // Layer: Editor UI (Draws the Inspector/Viewport)
    engine.PushLayer(new aether::EditorLayer());

    // 6. Loop
    engine.Run();

    return 0;
}
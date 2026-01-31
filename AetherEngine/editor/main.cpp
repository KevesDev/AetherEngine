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

// Include the Serializer
#include "../scene/SceneSerializer.h"

int main()
{
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

    // 3. Start Engine (Initializes VFS and Logging)
    aether::Engine engine(spec, settings);

    // --- 4. INITIALIZE WORLD STATE ---
    // The Editor needs a "Sandbox" world to edit.
    auto world = std::make_unique<aether::World>("Editor Sandbox");
    aether::Scene* scene = world->GetScene();

    if (scene) {
        // --- TEST: Create Data ---
        AETHER_CORE_INFO("--- SERIALIZER TEST START ---");

        auto player = scene->CreateEntity("Red Box");
        player.AddComponent<aether::SpriteComponent>(1.0f, 0.2f, 0.2f, 1.0f); // Red Color
        player.GetComponent<aether::TransformComponent>().X = 200.0f; // Move it slightly

        AETHER_CORE_INFO("1. Created Entity: {0} (Red Box)", (uint32_t)player.GetID());

        // --- TEST: Serialize (Save) ---
        aether::SceneSerializer serializer(scene);

        // Note: VFS maps "/assets" -> local "assets" folder.
        // Make sure the "assets" folder exists in your project directory!
        std::string path = "/assets/Level1.json";
        serializer.Serialize(path);

        // --- TEST: Destroy Data (Clear World) ---
        scene->DestroyEntity(player);
        AETHER_CORE_INFO("2. Destroyed Entity. World is empty.");

        // --- TEST: Deserialize (Load) ---
        serializer.Deserialize(path);

        // Verification: Check if entity exists (The Serializer logs "Deserialized Scene...")
        AETHER_CORE_INFO("3. Loaded Scene. Check for Red Box.");
        AETHER_CORE_INFO("--- SERIALIZER TEST END ---");
    }

    // Set the world active
    engine.SetWorld(std::move(world));
    // ---------------------------------

    // 5. Push Layers
    engine.PushOverlay(new aether::ImGuiLayer());
    engine.PushLayer(new aether::EditorLayer());

    // 6. Loop
    engine.Run();

    return 0;
}
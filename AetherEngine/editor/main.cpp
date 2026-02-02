#include <iostream>
#include <memory>
#include "../engine/core/Engine.h"
#include "../engine/core/EngineVersion.h"
#include "../engine/core/Log.h"
#include "../engine/core/Config.h"
#include "../engine/core/Layers/ImGuiLayer.h"
#include "layers/EditorLayer.h"
#include "../engine/scene/World.h"
#include "../engine/scene/SceneSerializer.h"
#include "../engine/core/VFS.h"

int main()
{
    // 0. Bootstrap Core Systems
    aether::Log::Init();
    aether::VFS::Mount("/assets", "assets");

    // 1. Identity
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    // 2. Preferences
    aether::WindowSettings settings;
    std::string startupScenePath;

    // Note the leading slash for VFS consistency
    if (!aether::Config::LoadBootConfig("/assets/editor_boot.json", settings, startupScenePath)) {
        settings.Title = "Aether Editor";
        settings.VSync = true;
    }
    else {
        settings.Title = "Aether Editor";
    }

    // 3. Start Engine
    auto engine = std::make_unique<aether::Engine>(spec, settings);

    // 4. Load Scene
    auto world = std::make_unique<aether::World>("Editor World");

    if (!startupScenePath.empty()) {
        std::string fullPath = startupScenePath;
        if (fullPath.find("/assets/") == std::string::npos) {
            fullPath = "/assets/" + fullPath;
        }

        if (!aether::VFS::ReadText(fullPath).empty()) {
            aether::SceneSerializer serializer(world->GetScene());
            serializer.Deserialize(fullPath);
            AETHER_CORE_INFO("Editor: Loaded startup scene: {0}", fullPath);
        }
        else {
            AETHER_CORE_WARN("Editor: Startup scene not found: {0}", fullPath);
        }
    }

    engine->SetWorld(std::move(world));

    // 5. Push Layers
    engine->PushOverlay(new aether::ImGuiLayer());
    engine->PushLayer(new aether::EditorLayer());

    // 6. Loop
    engine->Run();

    return 0;
}
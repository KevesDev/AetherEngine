#include <iostream>
#include <memory>
#include <filesystem>

#include "../engine/core/Engine.h"
#include "../engine/core/Log.h"
#include "../engine/core/Config.h"
#include "../engine/core/VFS.h"
#include "../engine/project/Project.h" 

#include "../engine/core/Layers/ImGuiLayer.h"
#include "layers/EditorLayer.h"
#include "../engine/scene/World.h"
#include "../engine/scene/SceneSerializer.h"

int main(int argc, char* argv[])
{
    // 1. Bootstrap Core
    aether::Log::Init();

    // 2. Resolve Project Path
    // If user drags a folder onto the .exe, argv[1] will be the path.
    std::filesystem::path projectPath;
    if (argc > 1) {
        projectPath = argv[1];
    }
    else {
        // FALLBACK: Look for "project.aether" in the current directory
        projectPath = "project.aether";
    }

    // 3. Load Project
    std::shared_ptr<aether::Project> activeProject;

    if (std::filesystem::exists(projectPath)) {
        activeProject = aether::Project::Load(projectPath);
    }
    else {
        // Create a default in-memory project if nothing is found (Safety Fallback)
        AETHER_CORE_WARN("Editor: No project found at {0}. Creating new default context.", projectPath.string());
        activeProject = aether::Project::New();
    }

    // 4. Mount VFS based on Project Config
    // This is the "Black Box" security. The engine now ONLY sees the Project's assets.
    auto assetPath = aether::Project::GetAssetDirectory();
    if (std::filesystem::exists(assetPath)) {
        aether::VFS::Mount("/assets", assetPath.string());
        AETHER_CORE_INFO("Editor: Mounted VFS '/assets' -> '{0}'", assetPath.string());
    }
    else {
        // Fallback for safety to prevent crash
        std::filesystem::create_directories("assets");
        aether::VFS::Mount("/assets", "assets");
    }

    // 5. Load Editor Preferences
    // We load this *after* VFS mount because editor_boot.json lives in the project assets
    aether::WindowSettings settings;
    std::string startupScenePath;

    if (!aether::Config::LoadBootConfig("/assets/editor_boot.json", settings, startupScenePath)) {
        settings.Title = "Aether Editor";
        settings.VSync = true;
    }
    // Override title to show active project
    settings.Title = "Aether Editor - " + activeProject->GetActiveConfig().Name;

    // 6. Start Engine
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    auto engine = std::make_unique<aether::Engine>(spec, settings);

    // 7. Load Start Scene
    auto world = std::make_unique<aether::World>("Editor World");

    // Priority: Project Config Start Scene > Editor Last Open Scene
    std::string sceneToLoad = activeProject->GetActiveConfig().StartScene;
    if (sceneToLoad.empty()) sceneToLoad = startupScenePath;

    if (!sceneToLoad.empty()) {
        // Ensure path has virtual root
        if (sceneToLoad.find("/assets/") == std::string::npos)
            sceneToLoad = "/assets/" + sceneToLoad;

        if (!aether::VFS::ReadText(sceneToLoad).empty()) {
            aether::SceneSerializer serializer(world->GetScene());
            serializer.Deserialize(sceneToLoad);
            AETHER_CORE_INFO("Editor: Loaded scene: {0}", sceneToLoad);
        }
    }

    engine->SetWorld(std::move(world));

    // 8. Push Layers
    engine->PushOverlay(new aether::ImGuiLayer());
    engine->PushLayer(new aether::EditorLayer());

    // 9. Run
    engine->Run();

    return 0;
}
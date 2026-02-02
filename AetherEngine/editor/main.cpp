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
#include "layers/ProjectHubLayer.h" 
#include "../engine/scene/World.h"
#include "../engine/scene/SceneSerializer.h"

int main(int argc, char* argv[])
{
    // 1. Initialize Logging First
    aether::Log::Init();

    // --- GLOBAL SAFETY NET START ---
    try {
        // 2. Mount Internal Content (Needed for Shaders)
        if (std::filesystem::exists("EngineContent")) {
            aether::VFS::Mount("/engine", "EngineContent");
        }
        else {
            AETHER_CORE_CRITICAL("CRITICAL MISSING DATA: 'EngineContent' folder not found. Working Dir: {}", std::filesystem::current_path().string());
            // We don't return -1 here yet to let the logger flush, but this is fatal.
        }

        // 3. Parse Args
        std::filesystem::path projectPath;
        bool openHub = true;

        if (argc > 1) {
            projectPath = argv[1];
            if (std::filesystem::exists(projectPath)) {
                openHub = false;
            }
        }

        // 4. Setup Window
        aether::WindowSettings settings;
        settings.Title = "Aether Hub";
        settings.Width = 1280;
        settings.Height = 720;
        settings.VSync = true;

        aether::EngineSpecification spec;
        spec.Name = "Aether Editor";
        spec.Type = aether::ApplicationType::Editor;

        auto engine = std::make_unique<aether::Engine>(spec, settings);
        auto world = std::make_unique<aether::World>("Editor World");
        engine->SetWorld(std::move(world));

        // 5. Push Overlays
        engine->PushOverlay(new aether::ImGuiLayer());

        // 6. Push Logic Layers
        if (openHub) {
            engine->PushLayer(new aether::ProjectHubLayer());
        }
        else {
            auto activeProject = aether::Project::Load(projectPath);
            if (activeProject) {
                auto assetPath = aether::Project::GetAssetDirectory();
                if (std::filesystem::exists(assetPath)) {
                    aether::VFS::Mount("/assets", assetPath.string());
                }
                engine->PushLayer(new aether::EditorLayer());
            }
            else {
                AETHER_CORE_ERROR("Failed to load project from args: {}", projectPath.string());
                // Fallback to Hub if load fails
                engine->PushLayer(new aether::ProjectHubLayer());
            }
        }

        // 7. Launch
        AETHER_CORE_INFO("Aether Engine Initialized. Starting Loop...");
        engine->Run();
    }
    // --- CATCH STANDARD EXCEPTIONS (e.g. std::filesystem, std::bad_alloc) ---
    catch (const std::exception& e) {
        AETHER_CORE_CRITICAL("FATAL CRASH: Unhandled Exception: {}", e.what());

        // Pause so you can read the error if running from VS without debugger
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    // --- CATCH UNKNOWN EXCEPTIONS ---
    catch (...) {
        AETHER_CORE_CRITICAL("FATAL CRASH: Unknown Exception occurred!");
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    // --- GLOBAL SAFETY NET END ---

    return 0;
}
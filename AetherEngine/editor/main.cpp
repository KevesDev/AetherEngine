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
    aether::Log::Init();

    try {
        if (std::filesystem::exists("EngineContent")) {
            aether::VFS::Mount("/engine", "EngineContent");
        }
        else {
            AETHER_CORE_CRITICAL("CRITICAL MISSING DATA: 'EngineContent' folder not found. Working Dir: {}", std::filesystem::current_path().string());
        }

        std::filesystem::path projectPath;
        bool openHub = true;

        if (argc > 1) {
            projectPath = argv[1];
            if (std::filesystem::exists(projectPath)) {
                openHub = false;
            }
        }

        aether::WindowProps settings;
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

        // 1. Push ImGui Overlay
        auto* imguiLayer = new aether::ImGuiLayer();
        engine->PushOverlay(imguiLayer);

        // Tell Engine this is the UI layer so it calls Begin()/End()
        engine->SetImGuiLayer(imguiLayer);

        // 2. Push Logic Layers
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
                engine->PushLayer(new aether::ProjectHubLayer());
            }
        }

        AETHER_CORE_INFO("Aether Engine Initialized. Starting Loop...");
        engine->Run();
    }
    catch (const std::exception& e) {
        AETHER_CORE_CRITICAL("FATAL CRASH: Unhandled Exception: {}", e.what());
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        return -1;
    }
    catch (...) {
        AETHER_CORE_CRITICAL("FATAL CRASH: Unknown Exception occurred!");
        std::cout << "Press ENTER to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    return 0;
}
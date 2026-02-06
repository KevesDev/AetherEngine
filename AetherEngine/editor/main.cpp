#include <iostream>
#include <memory>
#include <filesystem>
#include "../engine/core/Engine.h"
#include "../engine/core/Log.h"
#include "../engine/core/Config.h"
#include "../engine/core/VFS.h"
#include "../engine/asset/AssetManager.h"
#include "layers/ProjectHubLayer.h" 

int main(int argc, char* argv[])
{
    aether::Log::Init();

    try {
        AETHER_CORE_TRACE("Startup: Initializing VFS...");
        // Mount Engine Content
        if (std::filesystem::exists("EngineContent")) {
            aether::VFS::Mount("/engine", "EngineContent");
        }
        else {
            AETHER_CORE_CRITICAL("CRITICAL MISSING DATA: 'EngineContent' folder not found.");
        }

        aether::EngineSpecification spec;
        spec.Name = "Aether Editor";
        spec.Type = aether::ApplicationType::Client;
        spec.Width = 1280;
        spec.Height = 720;

        AETHER_CORE_TRACE("Startup: Creating Engine instance...");
        auto engine = std::make_unique<aether::Engine>(spec);

        // Centralized Logic: Always push Hub. 
        // If CLI args exist, pass them to Hub for auto-loading.
        auto* hubLayer = new aether::ProjectHubLayer();

        if (argc > 1) {
            std::filesystem::path projectPath = argv[1];
            if (std::filesystem::exists(projectPath)) {
                AETHER_CORE_TRACE("Startup: Auto-loading project from CLI args: {}", projectPath.string());
                hubLayer->SetAutoLoadProject(projectPath);
            }
        }

        engine->PushLayer(hubLayer);

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

    aether::AssetManager::Shutdown();
    return 0;
}
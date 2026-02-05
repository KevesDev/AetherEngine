#include "../engine/core/Engine.h"
#include "../engine/core/Config.h"
#include "../engine/core/Log.h"
#include "../engine/scene/SceneSerializer.h"
#include "../engine/scene/Scene.h"
#include "../engine/core/VFS.h"
#include <iostream>
#include <memory>

using namespace aether;

int main(int argc, char* argv[])
{
    // 0. Bootstrap Core Systems
    // We must init Logging and VFS *before* Config, because Config uses them!
    Log::Init();
    VFS::Mount("/assets", "assets");

    // 1. Load Configuration
    WindowProps windowSettings;
    std::string startupScenePath;

    // Note the leading slash: "/assets/boot.json" matches the mount point "/assets"
    if (!Config::LoadBootConfig("/assets/boot.json", windowSettings, startupScenePath)) {
        AETHER_CORE_WARN("Client: Failed to load specific boot config. Using defaults.");
    }

    // 2. Initialize Engine
    EngineSpecification spec;
    spec.Name = "Aether Client";
    spec.Type = ApplicationType::Client;
    spec.Width = windowSettings.Width;
    spec.Height = windowSettings.Height;

    auto engine = std::make_unique<Engine>(spec);

    // 3. Load the Startup Scene (Optional)
    // NOTE: The Scene is now managed by the application layer, not the Engine core.
    // This maintains the "Black Box" principle - the Engine doesn't know about Scenes.
    std::shared_ptr<Scene> activeScene = nullptr;

    if (!startupScenePath.empty()) {
        // Ensure path logic is consistent (add /assets/ if the JSON didn't include it)
        std::string scenePath = startupScenePath;
        if (scenePath.find("/assets/") == std::string::npos) {
            scenePath = "/assets/" + scenePath;
        }

        if (VFS::ReadText(scenePath).empty()) {
            AETHER_CORE_ERROR("Startup scene not found or empty: {0}", scenePath);
        }
        else {
            activeScene = std::make_shared<Scene>();
            SceneSerializer serializer(activeScene);

            // Convert VFS path to physical path for deserialization
            std::filesystem::path physicalPath;
            if (VFS::Resolve(scenePath, physicalPath)) {
                serializer.Deserialize(physicalPath);
                AETHER_CORE_INFO("Client: Loaded startup scene from {0}", scenePath);
            }
        }
    }
    else {
        AETHER_CORE_WARN("No startup scene defined in boot.json.");
    }

    // 4. Run Application
    engine->Run();

    return 0;
}
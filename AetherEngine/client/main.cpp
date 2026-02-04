#include "../engine/core/Engine.h"
#include "../engine/core/Config.h"
#include "../engine/core/Log.h"
#include "../engine/scene/SceneSerializer.h"
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

    auto engine = std::make_unique<Engine>(spec, windowSettings);

    // 3. Load the Startup Scene
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
            auto world = std::make_unique<World>("Runtime World");
            SceneSerializer serializer(world->GetScene());
            serializer.Deserialize(scenePath);
            engine->SetWorld(std::move(world));
        }
    }
    else {
        AETHER_CORE_WARN("No startup scene defined in boot.json.");
    }

    // 4. Run Application
    engine->Run();

    return 0;
}
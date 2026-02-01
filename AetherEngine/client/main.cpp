#include <iostream>
#include "../engine/core/Engine.h"
#include "../engine/core/EngineVersion.h"
#include "../engine/core/Log.h"
#include "../engine/core/VFS.h"
#include "../engine/scene/SceneSerializer.h"

// Vendor
#include "../engine/vendor/json.hpp"
using json = nlohmann::json;

int main()
{
    AETHER_CORE_INFO("Aether Client Wrapper starting...");

    // 2. Pre-Init VFS to read Boot Config
    // We need to mount assets early to find boot.json
    // TODO: In the future (Phase 4), this line changes to mount a .pak file.
    aether::VFS::Mount("/assets", "assets");

    std::string bootConfigPath = "/assets/boot.json";
    std::string bootContent = aether::VFS::ReadText(bootConfigPath);

    if (bootContent.empty())
    {
        AETHER_CORE_ERROR("CRITICAL: Could not load boot config from '{0}'", bootConfigPath);
        return -1;
    }

    // 3. Parse Boot Config
    json bootJson = json::parse(bootContent);

    std::string windowTitle = bootJson["Window"]["Title"];
    int width = bootJson["Window"]["Width"];
    int height = bootJson["Window"]["Height"];
    std::string startupScene = bootJson["StartupScene"];

    // 4. Setup Engine Spec
    aether::EngineSpecification spec;
    spec.Name = "Aether Client";
    spec.Type = aether::ApplicationType::Client; // Important: This prevents Editor UI from loading

    aether::WindowSettings settings;
    settings.Title = windowTitle;
    settings.Width = width;
    settings.Height = height;
    settings.Mode = aether::WindowMode::Windowed;
    settings.VSync = true;

    // 5. Initialize Engine
    // Note: Engine constructor will mount /assets again, but VFS handles duplicates/overrides gracefully.
    aether::Engine engine(spec, settings);

    // 6. Load the Game World
    auto world = std::make_unique<aether::World>("Runtime World");
    aether::Scene* scene = world->GetScene();

    if (scene)
    {
        std::string scenePath = "/assets/" + startupScene;
        AETHER_CORE_INFO("Loading Startup Scene: {0}", scenePath);

        aether::SceneSerializer serializer(scene);
        serializer.Deserialize(scenePath);
    }

    // Hand off world to Engine
    engine.SetWorld(std::move(world));

    // 7. Run!
    engine.Run();

    return 0;
}
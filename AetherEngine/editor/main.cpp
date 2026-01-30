#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Config.h"

int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Aether Editor starting up...");

    // 1. IDENTITY
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    // 2. PREFERENCES
    // Default override for Editor (we prefer it larger by default)
    aether::WindowSettings settings;
    settings.Width = 1600;
    settings.Height = 900;
    settings.Title = "Aether Editor";

    // Try to load saved preferences. 
    // Note: If editor.ini exists, it will overwrite the 1600x900 defaults above.
    aether::Config::Load("editor.ini", settings);

    // 3. START ENGINE
    aether::Engine engine(spec, settings);
    engine.Run();

    return 0;
}

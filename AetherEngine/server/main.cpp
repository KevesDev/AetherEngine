#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"

int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Aether Dedicated Server starting up...");

    // 1. ENGINE SPECIFICATION
    // The Type::Server flag tells the Engine constructor to run Headless (No Window).
    aether::EngineSpecification spec;
    spec.Name = "Aether Server";
    spec.Type = aether::ApplicationType::Server;

    // 2. WINDOW SETTINGS
    // We pass default settings. The Engine constructor will see 'Type::Server' 
    // and skip Window creation, so these values are effectively ignored.
    aether::WindowSettings settings;

    // 3. START ENGINE
    aether::Engine engine(spec, settings);
    engine.Run();

    return 0;
}

#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Config.h"

int main()
{
    // 1. IDENTITY
    aether::EngineSpecification spec;
    spec.Name = "Aether Client";
    spec.Type = aether::ApplicationType::Client;

    // 2. PREFERENCES (Loaded from disk)
    aether::WindowSettings settings;
    // Overwrite defaults with whatever is in the file
    aether::Config::Load("client.ini", settings);

    // 3. START ENGINE
    aether::Engine engine(spec, settings);
    engine.Run();

    return 0;
}
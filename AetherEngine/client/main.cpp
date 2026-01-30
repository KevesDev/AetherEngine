#include <iostream>
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Engine.h"

int main()
{
    // 1. DEFINE WHO I AM (Immutable)
    aether::EngineSpecification spec;
    spec.Name = "Aether Client";
    spec.Type = aether::ApplicationType::Client;

    // 2. DEFINE USER PREFERENCES (Mutable/Loadable)
    aether::WindowSettings settings;
    settings.Title = "Aether Game";
    settings.Width = 1280;
    settings.Height = 720;
    settings.VSync = true;

    // 3. START ENGINE
    aether::Engine engine(spec, settings);
    engine.Run();

    return 0;
}

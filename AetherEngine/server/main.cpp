#include <iostream>
#include "../engine/core/Engine.h"
#include "../engine/core/EngineVersion.h"
#include "../engine/core/Log.h"

int main()
{
    aether::Log::Init();
    AETHER_CORE_INFO("Aether Dedicated Server starting up...");

    // 1. ENGINE SPECIFICATION
    // The Type::Server flag tells the Engine constructor to run Headless (No Window).
    aether::EngineSpecification spec;
    spec.Name = "Aether Server";
    spec.Type = aether::ApplicationType::Server;
    spec.Width = 0;  // Ignored in headless mode
    spec.Height = 0; // Ignored in headless mode

    // 2. START ENGINE
    // Note: Server mode does not require WindowProps - the Engine handles headless initialization
    aether::Engine engine(spec);
    engine.Run();

    return 0;
}
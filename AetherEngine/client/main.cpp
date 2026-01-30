#include <iostream>
#include "EngineVersion.h"
#include "Log.h"
#include "Engine.h"

// Entry point for the Aether Client application.
// Right now it's just testing output.
int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Client starting up");

    // Start the core engine
    aether::Engine engine;
    engine.Run();
    return 0;
}

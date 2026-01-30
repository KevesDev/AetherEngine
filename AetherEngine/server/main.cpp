#include <iostream>
#include "EngineVersion.h"
#include "Log.h"
#include "Engine.h"

// Entry point for the Aether Server application.
// Right now it just prints the engine version to the console.
int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Server starting up");
    
    // Start the core engine
    aether::Engine engine("Server"); // Pass "Server" as the app name for the window title
    engine.Run();
    return 0;
}

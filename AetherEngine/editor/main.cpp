#include <iostream>
#include "EngineVersion.h"
#include "Log.h"
#include "Engine.h"

// Entry point for the Aether Editor application.
// Right now it just prints the engine version to the console.
int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Editor starting up");
    
    // Start the core engine
	aether::Engine engine("Editor"); // Pass "Editor" as the app name for the window title
    engine.Run();
    return 0;
}

#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Config.h"
#include "../core/Layers/ImGuiLayer.h" // <--- Don't forget this!

int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Aether Editor starting up...");

    // 1. IDENTITY
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    // 2. PREFERENCES
    aether::WindowSettings settings;
    settings.Width = 1600;
    settings.Height = 900;
    settings.Title = "Aether Editor";
    aether::Config::Load("editor.ini", settings);

    // 3. CREATE ENGINE
    aether::Engine engine(spec, settings);

    // 4. ADD IMGUI OVERLAY (MUST BE BEFORE RUN!)
    aether::ImGuiLayer* imguiLayer = new aether::ImGuiLayer();
    engine.PushOverlay(imguiLayer);

    // 5. START ENGINE
    engine.Run();

    return 0;
}
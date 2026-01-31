#include <iostream>
#include "../core/Engine.h"
#include "../core/EngineVersion.h"
#include "../core/Log.h"
#include "../core/Config.h"
#include "../core/Layers/ImGuiLayer.h"
#include "layers/EditorLayer.h"

int main()
{
    aether::Log::Write(aether::LogLevel::Info, "Aether Editor starting up...");

    // Identity and Preferences
    aether::EngineSpecification spec;
    spec.Name = "Aether Editor";
    spec.Type = aether::ApplicationType::Editor;

    aether::WindowSettings settings;
    settings.Width = 1280; // Ignored if Maximized
    settings.Height = 720;
    settings.Title = "Aether Editor";

    // Set the window to maximize automatically
    settings.Mode = aether::WindowMode::Maximized;

    // Load any saved settings (like Title or VSync)
    aether::Config::Load("editor.ini", settings);

    aether::Engine engine(spec, settings);

    // 1. Push ImGui Overlay (Handles the backend)
    engine.PushOverlay(new aether::ImGuiLayer());

    // 2. Push Editor Layer (Draws the UI)
    engine.PushLayer(new aether::EditorLayer());

    engine.Run();

    return 0;
}
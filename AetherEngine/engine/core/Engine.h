#pragma once
#include "../platform/Window.h"
#include "../events/Event.h"
#include "../events/ApplicationEvent.h"
#include <string>
#include <memory>

namespace aether {

    class Engine
    {

    // ENGINE SPECIFICATION (The "DNA" of the application)
    // These are hard-coded in main.cpp and cannot be changed by the user.
        enum class ApplicationType {
            Client,
            Server,
            Editor
        };

		// Holds basic info about the engine/application
        struct EngineSpecification {
            std::string Name = "Aether Engine";
            ApplicationType Type = ApplicationType::Client;
            // Future: CommandLineArgs, WorkingDirectory, etc.
        };

        // WINDOW SETTINGS (The "Preferences")
        // These could eventually be loaded from a "User.ini" file.
        struct WindowSettings {
            std::string Title = "Aether Application";
            uint32_t Width = 1280;
            uint32_t Height = 720;
            bool VSync = true;
            // Future: FullscreenMode, WindowDecoration, etc.
        };


    public:
		// Constructor - Takes EngineSpecification and optional WindowSettings
        Engine(const EngineSpecification& engineSpec, const WindowSettings& windowSettings = WindowSettings());
        virtual ~Engine();

        void Run();

        // This is the function that receives ALL events from the window
        void OnEvent(Event& e);

        // Getters so we can check "Am I a Server?" later
        const EngineSpecification& GetSpec() const { return m_Spec; }

    private:
        // Handles specific events (e.g., clicking X to close)
        bool OnWindowClose(WindowCloseEvent& e);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;

        // Store the spec for runtime checks (e.g. "If Server, don't play sound")
        EngineSpecification m_Spec;
    };

}
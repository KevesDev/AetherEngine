#include "Config.h"
#include "Log.h"
#include "VFS.h"
#include "../vendor/json.hpp"

using json = nlohmann::json;

namespace aether {

    bool Config::LoadBootConfig(const std::string& filepath, WindowSettings& outSettings, std::string& outStartupScene)
    {
        std::string content = VFS::ReadText(filepath);
        if (content.empty()) {
            Log::Write(LogLevel::Warning, "Config: " + filepath + " not found. Using Engine Defaults.");
            // Do NOT hardcode resolution here. 
            // WindowSettings (WindowProps) defaults to 1280x720 + Maximized in the constructor.
            // We just ensure the title is correct.
            outSettings.Title = "Aether Engine";
            outStartupScene = "";
            return false;
        }

        try {
            json bootJson = json::parse(content);

            if (bootJson.contains("Window")) {
                auto& win = bootJson["Window"];
                outSettings.Title = win.value("Title", "Aether Engine");
                outSettings.Width = win.value("Width", 1280);
                outSettings.Height = win.value("Height", 720);
                outSettings.VSync = win.value("VSync", true);

                // CRITICAL FIX: Load the WindowMode Enum (Cast from int)
                int modeInt = win.value("Mode", (int)WindowMode::Maximized);
                outSettings.Mode = static_cast<WindowMode>(modeInt);
            }

            outStartupScene = bootJson.value("StartupScene", "");

            Log::Write(LogLevel::Info, "Config: Loaded boot settings from " + filepath);
            return true;
        }
        catch (json::exception& e) {
            Log::Write(LogLevel::Error, "Config: JSON Parsing Error: " + std::string(e.what()));
            return false;
        }
    }

    void Config::SaveBootConfig(const std::string& filepath, const WindowSettings& settings, const std::string& startupScene)
    {
        json bootJson;
        bootJson["Window"]["Title"] = settings.Title;
        bootJson["Window"]["Width"] = settings.Width;
        bootJson["Window"]["Height"] = settings.Height;
        bootJson["Window"]["VSync"] = settings.VSync;

        // CRITICAL FIX: Save the WindowMode Enum
        bootJson["Window"]["Mode"] = (int)settings.Mode;

        bootJson["StartupScene"] = startupScene;

        std::string dump = bootJson.dump(4);
        VFS::WriteText(filepath, dump);
        Log::Write(LogLevel::Info, "Config: Saved boot settings to " + filepath);
    }
}
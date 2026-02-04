#include "Config.h"
#include "Log.h"
#include "VFS.h"
#include "ConfigValidator.h"
#include "../vendor/json.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;

namespace aether {

    bool Config::LoadBootConfig(const std::string& filepath, WindowProps& outSettings, std::string& outStartupScene) {
        std::string content;

        // 1. Try VFS (Virtual File System)
        if (filepath.find("/") == 0 || filepath.find("\\") == 0) {
            content = VFS::ReadText(filepath);
        }

        // 2. Try Physical Filesystem
        if (content.empty() && std::filesystem::exists(filepath)) {
            std::ifstream stream(filepath);
            std::stringstream strStream;
            strStream << stream.rdbuf();
            content = strStream.str();
        }

        // --- SAFETY FALLBACK 1: File Missing ---
        if (content.empty()) {
            Log::Write(LogLevel::Warning, "Config: '" + filepath + "' not found. Using Safe Defaults.");
            outSettings.Title = "Aether Engine";
            outSettings.Width = 1280;
            outSettings.Height = 720;
            outSettings.Mode = WindowMode::Maximized;
            outStartupScene = "";
            return false;
        }

        try {
            json configJson = json::parse(content);

            if (configJson.contains("Window")) {
                auto& win = configJson["Window"];
                outSettings.Title = win.value("Title", "Aether Engine");
                outSettings.Width = win.value("Width", 1280);
                outSettings.Height = win.value("Height", 720);
                outSettings.VSync = win.value("VSync", true);

                int modeInt = win.value("Mode", (int)WindowMode::Maximized);
                outSettings.Mode = static_cast<WindowMode>(modeInt);
            }

            outStartupScene = configJson.value("StartupScene", "");

            // --- SAFETY FALLBACK 2: Validate Data ---
            // This prevents "0x0" crashes or invalid window states
            ConfigValidator::ValidateWindowSettings(outSettings);

            Log::Write(LogLevel::Info, "Config: Loaded and Validated settings from " + filepath);
            return true;
        }
        catch (json::exception& e) {
            Log::Write(LogLevel::Error, "Config: JSON Parsing Error: " + std::string(e.what()) + ". Reverting to defaults.");

            // Reset to safe defaults if JSON is corrupt
            outSettings.Width = 1280;
            outSettings.Height = 720;
            outSettings.Mode = WindowMode::Maximized;
            return false;
        }
    }

    void Config::SaveBootConfig(const std::string& filepath, const WindowProps& settings, const std::string& startupScene) {
        json configJson;
        configJson["Window"]["Title"] = settings.Title;
        configJson["Window"]["Width"] = settings.Width;
        configJson["Window"]["Height"] = settings.Height;
        configJson["Window"]["VSync"] = settings.VSync;
        configJson["Window"]["Mode"] = (int)settings.Mode;
        configJson["StartupScene"] = startupScene;

        std::string dump = configJson.dump(4);
        std::ofstream out(filepath);
        out << dump;
        out.close();

        Log::Write(LogLevel::Info, "Config: Saved settings to " + filepath);
    }
}
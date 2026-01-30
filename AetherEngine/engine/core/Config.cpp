#include "Config.h"
#include "Log.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace aether {

    // Helper to trim whitespace from start/end of strings
    static std::string Trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        if (std::string::npos == first) return "";
        size_t last = str.find_last_not_of(" \t\r\n");
        return str.substr(first, (last - first + 1));
    }

    void Config::Load(const std::string& filepath, WindowSettings& outSettings)
    {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            Log::Write(LogLevel::Warning, "Config file not found: " + filepath + ". Using defaults.");
            // We optionally save a fresh default file so the user has one to edit next time
            Save(filepath, outSettings);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // 1. Remove comments
            size_t commentPos = line.find_first_of("#;");
            if (commentPos != std::string::npos) line = line.substr(0, commentPos);

            // 2. Find Key=Value split
            size_t delimiterPos = line.find('=');
            if (delimiterPos == std::string::npos) continue; // Skip invalid lines

            // 3. Extract and clean
            std::string key = Trim(line.substr(0, delimiterPos));
            std::string value = Trim(line.substr(delimiterPos + 1));

            // 4. Map to Struct
            if (key == "Title")       outSettings.Title = value;
            else if (key == "Width")  outSettings.Width = std::stoi(value);
            else if (key == "Height") outSettings.Height = std::stoi(value);
            else if (key == "VSync")  outSettings.VSync = (value == "true" || value == "1");
        }

        Log::Write(LogLevel::Info, "Loaded config from " + filepath);
    }

    void Config::Save(const std::string& filepath, const WindowSettings& settings)
    {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            Log::Write(LogLevel::Error, "Failed to save config to " + filepath);
            return;
        }

        file << "# Aether Engine Configuration\n";
        file << "Title=" << settings.Title << "\n";
        file << "Width=" << settings.Width << "\n";
        file << "Height=" << settings.Height << "\n";
        file << "VSync=" << (settings.VSync ? "true" : "false") << "\n";

        Log::Write(LogLevel::Info, "Saved config to " + filepath);
    }
}
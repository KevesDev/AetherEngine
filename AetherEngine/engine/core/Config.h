#pragma once
#include "Engine.h"
#include <string>
#include <filesystem>

namespace aether {

    class Config
    {
    public:
        // Unified JSON Loader
        // Returns true if successful, false if defaults were used.
        static bool LoadBootConfig(const std::string& filepath, WindowSettings& outSettings, std::string& outStartupScene);

        static void SaveBootConfig(const std::string& filepath, const WindowSettings& settings, const std::string& startupScene);
    };

}
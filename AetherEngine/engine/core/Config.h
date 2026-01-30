#pragma once
#include "Engine.h"
#include <string>

namespace aether {

	// Static helper class for loading/saving configuration files.
    class Config
    {
    public:
        // Tries to load settings from 'filepath'. 
        // If file doesn't exist or errors occur, it leaves 'outSettings' as defaults.
        static void Load(const std::string& filepath, WindowSettings& outSettings);

        // Saves current settings to 'filepath'. 
        // Useful if we add an "Apply & Save" button in the options menu later.
        static void Save(const std::string& filepath, const WindowSettings& settings);
    };

}
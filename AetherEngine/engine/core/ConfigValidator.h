#pragma once
#include "../platform/Window.h"
#include <string>
#include <algorithm>
#include <cctype>

namespace aether {

    class ConfigValidator {
    public:
        // Templated to accept WindowProps or WindowSettings (duck typing)
        template<typename T>
        static void ValidateWindowSettings(T& settings) {

            // 1. Sanitize Resolution
            // Prevent 0x0 or negative-like overflow values
            if (settings.Width < 640) settings.Width = 1280;
            if (settings.Height < 480) settings.Height = 720;

            // 2. Sanitize Mode
            // If the integer value cast from JSON is garbage, reset to Maximized
            if (settings.Mode != WindowMode::Windowed &&
                settings.Mode != WindowMode::Borderless &&
                settings.Mode != WindowMode::Fullscreen &&
                settings.Mode != WindowMode::Maximized)
            {
                settings.Mode = WindowMode::Maximized;
            }

            // 3. Safety Fallback
            // If we detect a "zeroed out" config (crash recovery), force safe defaults
            if (settings.Width == 0 || settings.Height == 0) {
                settings.Width = 1280;
                settings.Height = 720;
                settings.Mode = WindowMode::Maximized;
            }
        }

        // Sanitize strings (remove illegal chars for file paths)
        static std::string SanitizeName(const std::string& name) {
            std::string safe = name;
            // Remove any non-alphanumeric chars (allow spaces, underscores, dashes)
            safe.erase(std::remove_if(safe.begin(), safe.end(), [](char c) {
                return !isalnum(c) && c != ' ' && c != '_' && c != '-';
                }), safe.end());

            if (safe.empty()) return "Untitled";
            if (safe.length() > 64) safe = safe.substr(0, 64);

            return safe;
        }
    };
}
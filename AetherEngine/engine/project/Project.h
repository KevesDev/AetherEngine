#pragma once

#include <string>
#include <filesystem>
#include <memory>
#include "../core/Log.h"

namespace aether {

    struct ProjectConfig
    {
        std::string Name = "Untitled";
        std::string StartScene;

        // The folder containing assets relative to the project root (usually "Assets")
        std::filesystem::path AssetDirectory = "Assets";

        // Folder for script binaries (Forward compatibility)
        std::filesystem::path ScriptModulePath = "Scripts/Binaries";
    };
    /* This defines the data structure for a project. 
    *  It uses std::filesystem to ensuring paths work 
    *  correctly on Windows, Linux, and macOS.
    */
    class Project
    {
    public:
        // --- Getters ---
        static const std::filesystem::path& GetProjectDirectory() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return s_ActiveProject->m_ProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
        }

        static ProjectConfig& GetActiveConfig() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return s_ActiveProject->m_Config;
        }

        static std::shared_ptr<Project> GetActive() { return s_ActiveProject; }

        // --- Core API ---
        static std::shared_ptr<Project> New();
        static std::shared_ptr<Project> Load(const std::filesystem::path& path);
        static bool SaveActive(const std::filesystem::path& path);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;

        inline static std::shared_ptr<Project> s_ActiveProject;
    };
}
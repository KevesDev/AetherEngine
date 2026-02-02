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
        std::filesystem::path AssetDirectory = "Assets";
        std::filesystem::path ScriptModulePath = "Scripts/Binaries";
    };

    class Project
    {
    public:
        // --- Validation API ---
        static bool IsValidName(const std::string& name);

        // --- Getters ---
        static const std::filesystem::path& GetProjectDirectory() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return s_ActiveProject->m_ProjectDirectory;
        }

        static std::filesystem::path GetAssetDirectory() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory;
        }

        static std::filesystem::path GetSettingsDirectory() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return GetProjectDirectory() / "ProjectSettings";
        }

        static ProjectConfig& GetActiveConfig() {
            AETHER_ASSERT(s_ActiveProject, "No active project!");
            return s_ActiveProject->m_Config;
        }

        static std::shared_ptr<Project> GetActive() { return s_ActiveProject; }

        // --- Core API ---
        static std::shared_ptr<Project> New();
        static std::shared_ptr<Project> Load(const std::filesystem::path& path);
        static std::shared_ptr<Project> Create(const std::filesystem::path& path);
        static bool SaveActive(const std::filesystem::path& path);

    private:
        ProjectConfig m_Config;
        std::filesystem::path m_ProjectDirectory;
        inline static std::shared_ptr<Project> s_ActiveProject;

        // Allow Serializer to access private m_Config directly
        friend class ProjectSerializer;
    };
}
#include "Project.h"
#include "ProjectSerializer.h"
#include "../core/Log.h"
#include "../vendor/json.hpp"
#include <fstream>
#include <filesystem>
#include <algorithm> // Required for validation logic

using json = nlohmann::json;

namespace aether {

    // --- Validation Implementation ---
    bool Project::IsValidName(const std::string& name)
    {
        // 1. Check Empty
        if (name.empty()) return false;

        // 2. Check Length
        if (name.length() > 255) return false;

        // 3. Check Characters (Alphanumeric Only)
        return std::all_of(name.begin(), name.end(), [](char c) {
            return (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9');
            });
    }

    std::shared_ptr<Project> Project::New()
    {
        s_ActiveProject = std::make_shared<Project>();
        return s_ActiveProject;
    }

    std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
    {
        std::shared_ptr<Project> project = std::make_shared<Project>();
        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path)) {
            project->m_ProjectDirectory = path.parent_path();
            s_ActiveProject = project;
            return s_ActiveProject;
        }
        return nullptr;
    }

    std::shared_ptr<Project> Project::Create(const std::filesystem::path& path)
    {
        // SECURITY CHECK: Validate Name before touching filesystem
        std::string projectName = path.stem().string();
        if (!IsValidName(projectName)) {
            AETHER_CORE_ERROR("Project Creation Failed: Invalid Name '{0}'. Must be alphanumeric and non-empty.", projectName);
            return nullptr;
        }

        std::filesystem::path projectRoot = path.parent_path();

        // 1. Generate Folders
        if (!std::filesystem::exists(projectRoot)) std::filesystem::create_directories(projectRoot);
        std::filesystem::create_directories(projectRoot / "Assets");
        std::filesystem::create_directories(projectRoot / "ProjectSettings");

        // 2. Generate boot.json
        json boot;
        boot["Window"]["Title"] = projectName;
        boot["Window"]["Width"] = 1280;
        boot["Window"]["Height"] = 720;
        boot["Window"]["VSync"] = true;
        boot["Window"]["Mode"] = 3; // Maximized
        boot["StartupScene"] = "";

        std::ofstream bootOut(projectRoot / "ProjectSettings/boot.json");
        bootOut << boot.dump(4);
        bootOut.close();

        // 3. Generate Manifest
        std::shared_ptr<Project> project = std::make_shared<Project>();
        project->m_Config.Name = projectName;
        project->m_Config.AssetDirectory = "Assets";

        s_ActiveProject = project;
        project->SaveActive(path);

        return project;
    }

    bool Project::SaveActive(const std::filesystem::path& path)
    {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path)) {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();
            return true;
        }
        return false;
    }
}
#include "Project.h"
#include "ProjectSerializer.h"
#include "../core/Log.h"
#include "../core/ConfigValidator.h"
#include "../vendor/json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

namespace aether {

    // --- Core Lifecycle ---

    std::shared_ptr<Project> Project::New()
    {
        s_ActiveProject = std::make_shared<Project>();
        return s_ActiveProject;
    }

    std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
    {
        std::shared_ptr<Project> project = std::make_shared<Project>();

        // This will use the Binary BSON deserializer
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
        // 1. Validate & Sanitize Name
        std::string rawName = path.stem().string();
        std::string safeName = ConfigValidator::SanitizeName(rawName);

        if (safeName != rawName) {
            AETHER_CORE_WARN("Project Name sanitized from '{0}' to '{1}'", rawName, safeName);
        }

        std::filesystem::path projectRoot = path.parent_path();

        // 2. Generate Folders
        if (!std::filesystem::exists(projectRoot)) std::filesystem::create_directories(projectRoot);
        std::filesystem::create_directories(projectRoot / "Assets");
        std::filesystem::create_directories(projectRoot / "ProjectSettings");

        // 3. Generate game.config (The Runtime Config - JSON)
        // We use safe defaults (Maximized) so it guarantees usability on any screen
        json config;
        config["Window"]["Title"] = safeName;
        config["Window"]["Width"] = 1280;
        config["Window"]["Height"] = 720;
        config["Window"]["VSync"] = true;
        // WindowMode::Maximized is usually index 3 (Windowed=0, Borderless=1, Fullscreen=2, Maximized=3)
        config["Window"]["Mode"] = 3;
        config["StartupScene"] = "";

        std::ofstream configOut(projectRoot / "ProjectSettings/game.config");
        configOut << config.dump(4);
        configOut.close();

        // 4. Create Project Instance
        std::shared_ptr<Project> project = std::make_shared<Project>();
        project->m_Config.Name = safeName;
        project->m_Config.AssetDirectory = "Assets";

        s_ActiveProject = project;

        // 5. Save .aether (Now uses Binary BSON)
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

    // Helper proxy
    bool Project::IsValidName(const std::string& name) {
        return name == ConfigValidator::SanitizeName(name);
    }
}
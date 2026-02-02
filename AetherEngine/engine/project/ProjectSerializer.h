#pragma once
#include "Project.h"
#include <memory>
#include <string>
#include <filesystem>

namespace aether {

    class ProjectSerializer
    {
    public:
        ProjectSerializer(std::shared_ptr<Project> project);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

        // Static helper to check version without loading the whole project
        static bool GetProjectVersion(const std::filesystem::path& filepath, std::string& outVersion);

    private:
        std::shared_ptr<Project> m_Project;
    };
}
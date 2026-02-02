#pragma once
#include "Project.h"

namespace aether {
    /* This handles reading and writing the project.aether JSON file. 
    *  We separate it from the Project class to keep the data logic clean 
    *  (Single Responsibility Principle).
    */

    class ProjectSerializer
    {
    public:
        ProjectSerializer(std::shared_ptr<Project> project);

        bool Serialize(const std::filesystem::path& filepath);
        bool Deserialize(const std::filesystem::path& filepath);

    private:
        std::shared_ptr<Project> m_Project;
    };
}
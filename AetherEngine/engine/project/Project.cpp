#include "Project.h"
#include "ProjectSerializer.h"
#include "../core/Log.h"

namespace aether {
    /* This implements the active project management. 
    *  It acts as the central hub that holds the configuration in memory.
    */
    std::shared_ptr<Project> Project::New()
    {
        s_ActiveProject = std::make_shared<Project>();
        return s_ActiveProject;
    }

    std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
    {
        std::shared_ptr<Project> project = std::make_shared<Project>();

        ProjectSerializer serializer(project);
        if (serializer.Deserialize(path))
        {
            // The project root is the directory containing the project.aether file
            project->m_ProjectDirectory = path.parent_path();
            s_ActiveProject = project;

            AETHER_CORE_INFO("Project: Loaded '{0}' successfully.", project->m_Config.Name);
            return s_ActiveProject;
        }

        AETHER_CORE_ERROR("Project: Failed to load project at '{0}'", path.string());
        return nullptr;
    }

    bool Project::SaveActive(const std::filesystem::path& path)
    {
        ProjectSerializer serializer(s_ActiveProject);
        if (serializer.Serialize(path))
        {
            s_ActiveProject->m_ProjectDirectory = path.parent_path();
            return true;
        }
        return false;
    }
}
#pragma once

#include "../../engine/core/Layers/Layer.h"
#include <filesystem>

namespace aether {

    class ProjectHubLayer : public Layer
    {
    public:
        ProjectHubLayer();
        virtual ~ProjectHubLayer() = default;

        virtual void OnAttach() override;
        virtual void OnImGuiRender() override;

        // Allows Main to request an immediate project load (CLI args)
        void SetAutoLoadProject(const std::filesystem::path& path) { m_AutoLoadProject = path; }

    private:
        void RenderFileBrowser();
        void CreateProject(const std::string& pathStr);
        void LoadProject(const std::string& pathStr);

    private:
        std::filesystem::path m_CurrentDirectory;
        char m_NewProjectNameBuffer[256] = "";
        char m_FileSearchBuffer[256] = "";
        bool m_ShowFileBrowser = false;

        std::filesystem::path m_AutoLoadProject;
    };

}
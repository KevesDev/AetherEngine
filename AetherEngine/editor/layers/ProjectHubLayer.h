#pragma once
#include "../../engine/core/Layers/Layer.h"
#include <filesystem>
#include <string>

namespace aether {

    class ProjectHubLayer : public Layer {
    public:
        ProjectHubLayer();
        virtual void OnAttach() override;
        virtual void OnImGuiRender() override;

    private:
        void CreateProject(const std::string& path);
        void LoadProject(const std::string& path);

        // UI State
        char m_NewProjectNameBuffer[256] = "";

        // File Browser State
        bool m_ShowFileBrowser = false;
        std::filesystem::path m_CurrentDirectory;
        char m_FileSearchBuffer[256] = "";

        // Helper to render the popup
        void RenderFileBrowser();
    };
}
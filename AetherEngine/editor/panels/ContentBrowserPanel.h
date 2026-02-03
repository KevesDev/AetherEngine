#pragma once
#include "EditorPanel.h"
#include <filesystem>

namespace aether {

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_BaseDirectory;
    };
}
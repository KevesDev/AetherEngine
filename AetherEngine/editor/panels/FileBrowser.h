#pragma once

#include <string>
#include <filesystem>
#include <vector>

namespace aether {

    class FileBrowser
    {
    public:
        FileBrowser();

        // Opens the popup
        void Open();

        // Configuration
        void SetTitle(const std::string& title);
        void SetSearchHint(const std::string& hint);

        // Example: { ".png", ".jpg" } - If empty, all files are selectable
        void SetFileExtensions(const std::vector<std::string>& extensions);

        // Returns true if a file was selected. 'outPath' is populated.
        // Must be called every frame to render the popup if open.
        bool Render(std::filesystem::path& outPath);

        bool IsOpen() const { return m_IsOpen; }

    private:
        bool m_IsOpen = false;
        std::string m_Title = "Open File";
        std::string m_SearchHint = "Search files...";

        std::filesystem::path m_CurrentDirectory;
        char m_SearchBuffer[256] = "";

        std::vector<std::string> m_AllowedExtensions;
    };
}
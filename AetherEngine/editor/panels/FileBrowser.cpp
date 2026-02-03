#include "FileBrowser.h"
#include "../../engine/core/Theme.h" // Import the centralized theme
#include <imgui.h>
#include <algorithm>
#include <system_error>

namespace aether {

    FileBrowser::FileBrowser()
    {
        // Default to current working directory
        m_CurrentDirectory = std::filesystem::current_path();
    }

    void FileBrowser::Open()
    {
        m_IsOpen = true;
        m_SearchBuffer[0] = '\0';
        ImGui::OpenPopup(m_Title.c_str());
    }

    void FileBrowser::SetTitle(const std::string& title)
    {
        m_Title = title;
    }

    void FileBrowser::SetSearchHint(const std::string& hint)
    {
        m_SearchHint = hint;
    }

    void FileBrowser::SetFileExtensions(const std::vector<std::string>& extensions)
    {
        m_AllowedExtensions = extensions;
        // Ensure extensions are lowercase for consistency
        for (auto& ext : m_AllowedExtensions) {
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        }
    }

    bool FileBrowser::Render(std::filesystem::path& outPath)
    {
        if (!m_IsOpen) return false;

        bool fileSelected = false;
        Theme theme; // Access unified colors

        // Center the window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(700, 500));

        if (ImGui::BeginPopupModal(m_Title.c_str(), &m_IsOpen, ImGuiWindowFlags_NoResize))
        {
            // --- Header: Navigation & Path ---
            if (ImGui::Button("Up Level", ImVec2(80, 0))) {
                if (m_CurrentDirectory.has_parent_path())
                    m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }

            ImGui::SameLine();
            ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);

            // Path: Use Primary Accent (Lavender) to make it readable but distinct
            ImGui::TextColored(theme.AccentPrimary, "Dir: %s", m_CurrentDirectory.string().c_str());

            ImGui::PopTextWrapPos();

            ImGui::Separator();

            // --- Search Bar ---
            ImGui::InputTextWithHint("##FileSearch", m_SearchHint.c_str(), m_SearchBuffer, 256);

            // --- File List ---
            // Use Panel Background for the list area to create depth
            ImGui::PushStyleColor(ImGuiCol_ChildBg, theme.PanelBg);
            ImGui::BeginChild("FileList", ImVec2(0, 350), true);

            std::error_code ec;
            auto iterator = std::filesystem::directory_iterator(m_CurrentDirectory, ec);

            if (!ec) {
                // 1. Directories First
                for (const auto& entry : iterator) {
                    if (entry.is_directory()) {
                        std::string dirName = "[DIR]  " + entry.path().filename().string();

                        // Directories use standard Text color
                        if (ImGui::Selectable(dirName.c_str())) {
                            m_CurrentDirectory = entry.path();
                        }
                    }
                }

                // 2. Files
                // Reset iterator to iterate again for files
                iterator = std::filesystem::directory_iterator(m_CurrentDirectory, ec);

                for (const auto& entry : iterator) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        std::string searchStr = m_SearchBuffer;

                        bool matchesSearch = searchStr.empty() || filename.find(searchStr) != std::string::npos;
                        bool matchesExt = m_AllowedExtensions.empty();

                        // Check extension filter
                        if (!matchesExt) {
                            for (const auto& allowed : m_AllowedExtensions) {
                                if (ext == allowed) {
                                    matchesExt = true;
                                    break;
                                }
                            }
                        }

                        if (matchesSearch) {
                            if (matchesExt) {
                                // Valid File: Use AccentCyan (Teal) to indicate "Good to go"
                                ImGui::PushStyleColor(ImGuiCol_Text, theme.AccentCyan);
                                std::string label = "  " + filename;
                                if (ImGui::Selectable(label.c_str())) {
                                    outPath = entry.path();
                                    fileSelected = true;
                                    m_IsOpen = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::PopStyleColor();
                            }
                            else {
                                // Invalid File: Use TextMuted (Grey)
                                ImGui::PushStyleColor(ImGuiCol_Text, theme.TextMuted);
                                ImGui::Text("  %s", filename.c_str());
                                ImGui::PopStyleColor();
                            }
                        }
                    }
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor(); // Pop ChildBg

            // --- Footer ---
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_IsOpen = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return fileSelected;
    }
}
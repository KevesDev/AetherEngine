#include "ProjectHubLayer.h"
#include "EditorLayer.h"
#include "../../engine/core/Engine.h"
#include "../../engine/project/Project.h"
#include "../../engine/project/ProjectSerializer.h"
#include "../../engine/core/Theme.h"
#include "../../engine/core/EngineVersion.h"
#include "../../engine/core/VFS.h"
#include "../../engine/asset/AssetManager.h" // [Fix] Required for Init
#include <imgui.h>
#include <algorithm>

namespace aether {

    ProjectHubLayer::ProjectHubLayer() : Layer("ProjectHub") {
        m_CurrentDirectory = std::filesystem::current_path();
    }

    void ProjectHubLayer::OnAttach() {
        Theme theme;
        ThemeManager::ApplyTheme(theme);

        // [Fix] Handle CLI auto-load request immediately
        if (!m_AutoLoadProject.empty()) {
            LoadProject(m_AutoLoadProject.string());
        }
    }

    void ProjectHubLayer::OnImGuiRender() {
        // ... (GUI Render Code Omitted for brevity - it remains identical to your file)
        // Note: For the actual implementation, paste your original OnImGuiRender code here.
        // It is unchanged except it relies on the internal methods below.

        // (Existing GUI code...)
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(40, 40));
        ImGui::Begin("Project Hub", nullptr, flags);
        ImGui::PopStyleVar();

        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
        ImGui::SetWindowFontScale(2.5f);
        ImGui::TextColored(ImVec4(0.725f, 0.549f, 1.0f, 1.0f), "Aether Engine");
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        std::string versionStr = "Version " + EngineVersion::ToString();
        ImGui::TextColored(ImVec4(0.604f, 0.631f, 0.675f, 1.0f), versionStr.c_str());

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.086f, 0.106f, 0.133f, 1.0f));
        ImGui::BeginChild("NewProjectCard", ImVec2(400, 200), true, ImGuiWindowFlags_None);
        {
            ImGui::SetCursorPos(ImVec2(20, 20));
            ImGui::Text("CREATE NEW PROJECT");
            ImGui::SetCursorPos(ImVec2(20, 45));
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Start a fresh Aether project.");

            ImGui::SetCursorPos(ImVec2(20, 80));
            ImGui::SetNextItemWidth(360);
            ImGui::InputText("##ProjectName", m_NewProjectNameBuffer, sizeof(m_NewProjectNameBuffer));

            ImGui::SetCursorPos(ImVec2(20, 130));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.725f, 0.549f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.65f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.059f, 0.071f, 0.090f, 1.0f));

            if (ImGui::Button("CREATE PROJECT", ImVec2(360, 45))) {
                std::string name = m_NewProjectNameBuffer;
                if (Project::IsValidName(name)) {
                    std::filesystem::path path = "Projects";
                    path /= name;
                    path /= name + ".aether";
                    CreateProject(path.string());
                }
                else {
                    ImGui::OpenPopup("Invalid Name");
                }
            }
            ImGui::PopStyleColor(3);

            if (ImGui::BeginPopupModal("Invalid Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Project names must be alphanumeric (A-Z, 0-9)\nand cannot be empty.");
                ImGui::Separator();
                if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.086f, 0.106f, 0.133f, 1.0f));
        ImGui::BeginChild("LoadProjectCard", ImVec2(400, 200), true, ImGuiWindowFlags_None);
        {
            ImGui::SetCursorPos(ImVec2(20, 20));
            ImGui::Text("OPEN EXISTING PROJECT");
            ImGui::SetCursorPos(ImVec2(20, 45));
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Browse for an .aether file.");

            ImGui::SetCursorPos(ImVec2(20, 130));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.7f, 0.8f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.8f, 0.9f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

            if (ImGui::Button("BROWSE FILES...", ImVec2(360, 45))) {
                m_ShowFileBrowser = true;
                m_FileSearchBuffer[0] = '\0';
            }
            ImGui::PopStyleColor(3);
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::End();

        if (m_ShowFileBrowser) {
            ImGui::OpenPopup("Open Project");
            RenderFileBrowser();
        }
    }

    void ProjectHubLayer::RenderFileBrowser() {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(700, 500));

        if (ImGui::BeginPopupModal("Open Project", &m_ShowFileBrowser, ImGuiWindowFlags_NoResize)) {

            if (ImGui::Button("Up Level", ImVec2(80, 0))) {
                if (m_CurrentDirectory.has_parent_path())
                    m_CurrentDirectory = m_CurrentDirectory.parent_path();
            }
            ImGui::SameLine();
            ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Dir: %s", m_CurrentDirectory.string().c_str());
            ImGui::PopTextWrapPos();

            ImGui::Separator();
            ImGui::InputTextWithHint("##FileSearch", "Search .aether files...", m_FileSearchBuffer, 256);

            ImGui::BeginChild("FileList", ImVec2(0, 350), true);

            std::error_code ec;
            auto iterator = std::filesystem::directory_iterator(m_CurrentDirectory, ec);

            if (!ec) {
                // Directories
                for (const auto& entry : iterator) {
                    if (entry.is_directory()) {
                        std::string dirName = "[DIR]  " + entry.path().filename().string();
                        if (ImGui::Selectable(dirName.c_str())) {
                            m_CurrentDirectory = entry.path();
                        }
                    }
                }

                // Files
                iterator = std::filesystem::directory_iterator(m_CurrentDirectory, ec);
                for (const auto& entry : iterator) {
                    if (entry.is_regular_file()) {
                        std::string filename = entry.path().filename().string();
                        std::string searchStr = m_FileSearchBuffer;

                        bool matchesSearch = searchStr.empty() || filename.find(searchStr) != std::string::npos;
                        bool isProject = entry.path().extension() == ".aether";

                        if (isProject && matchesSearch) {
                            std::string projVersion;
                            bool versionOk = false;

                            if (ProjectSerializer::GetProjectVersion(entry.path(), projVersion)) {
                                versionOk = (projVersion == EngineVersion::ToString());
                            }

                            if (versionOk) {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
                                std::string label = "[PROJ] " + filename;
                                if (ImGui::Selectable(label.c_str())) {
                                    LoadProject(entry.path().string());
                                    m_ShowFileBrowser = false;
                                    ImGui::CloseCurrentPopup();
                                }
                                ImGui::PopStyleColor();
                            }
                            else {
                                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
                                std::string label = "[PROJ] " + filename + " (Incompatible v" + projVersion + ")";
                                ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_Disabled);
                                ImGui::PopStyleColor();
                            }
                        }
                        else if (!isProject && matchesSearch) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
                            ImGui::Text("       %s", filename.c_str());
                            ImGui::PopStyleColor();
                        }
                    }
                }
            }
            ImGui::EndChild();

            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                m_ShowFileBrowser = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void ProjectHubLayer::CreateProject(const std::string& pathStr) {
        if (Project::Create(pathStr)) {
            auto assetPath = Project::GetAssetDirectory();
            if (std::filesystem::exists(assetPath)) VFS::Mount("/assets", assetPath.string());

            // Initialize Asset Manager (prevents crash on import)
            AssetManager::Init();

            Engine::Get().PushLayer(new EditorLayer());
            Engine::Get().PopLayer(this);
        }
    }

    void ProjectHubLayer::LoadProject(const std::string& pathStr) {
        if (Project::Load(pathStr)) {
            auto assetPath = Project::GetAssetDirectory();
            if (std::filesystem::exists(assetPath)) VFS::Mount("/assets", assetPath.string());

            // Initialize Asset Manager (prevents crash on import)
            AssetManager::Init();

            Engine::Get().PushLayer(new EditorLayer());
            Engine::Get().PopLayer(this);
        }
        else {
            AETHER_CORE_ERROR("Failed to load project: {0}", pathStr);
        }
    }
}
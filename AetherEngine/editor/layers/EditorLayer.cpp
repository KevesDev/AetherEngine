#include "EditorLayer.h"
#include "../../engine/core/Engine.h" 
#include "../../engine/scene/World.h"
#include "../../engine/core/Theme.h"
#include "../../engine/project/Project.h"
#include "../../engine/project/ProjectSerializer.h"
#include "../../engine/asset/AssetManager.h"
#include "../../engine/input/Input.h"       
#include "../../engine/input/KeyCodes.h"    
#include "../EditorResources.h"
#include "../panels/TextureViewerPanel.h"
#include "../commands/CommandHistory.h"     

#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>
#include <glad/glad.h>

namespace aether {

    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    void EditorLayer::OnAttach()
    {
        if (!ImGui::GetCurrentContext()) {
            AETHER_CORE_ERROR("EditorLayer attached but ImGui Context is missing!");
            AETHER_ASSERT(false, "ImGui Context Missing");
        }

        // Ensure events propagate through ImGuiLayer to this layer
        Engine::Get().GetImGuiLayer()->SetBlockEvents(false);

        Theme theme;
        ThemeManager::ApplyTheme(theme);

        auto settingsDir = Project::GetSettingsDirectory();
        if (!std::filesystem::exists(settingsDir)) {
            std::filesystem::create_directories(settingsDir);
        }

        m_IniFilePath = (settingsDir / "editor.ini").string();
        ImGui::GetIO().IniFilename = m_IniFilePath.c_str();
        ImGui::LoadIniSettingsFromDisk(m_IniFilePath.c_str());

        m_ConfigFilePath = (settingsDir / "editor_config.json").string();
        LoadSettings();

        std::string title = Project::GetActiveConfig().Name + " - Aether Editor";
        Engine::Get().GetWindow().SetTitle(title);

        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        EditorResources::Init();
        AssetManager::Init();

        m_ContentBrowserPanel.SetOnAssetOpenedCallback([this](const std::filesystem::path& path)
            {
                OpenAsset(path);
            });

        AETHER_CORE_INFO("Editor Initialized. Layout: {}", m_IniFilePath);
    }

    void EditorLayer::OnDetach()
    {
        if (ImGui::GetCurrentContext()) {
            ImGui::SaveIniSettingsToDisk(ImGui::GetIO().IniFilename);
        }
        SaveSettings();
        AssetManager::Shutdown();
        EditorResources::Shutdown();
    }

    void EditorLayer::LoadSettings()
    {
        m_Settings.Deserialize(m_ConfigFilePath);
        m_ContentBrowserPanel.SetShowRawAssets(m_Settings.ShowRawAssets);
    }

    void EditorLayer::SaveSettings()
    {
        m_Settings.Serialize(m_ConfigFilePath);
    }

    void EditorLayer::OnUpdate(TimeStep ts)
    {
        if (!m_Framebuffer) return;

        if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
            m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (spec.Width != (uint32_t)m_ViewportSize.x || spec.Height != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        if (m_ViewportFocused) m_EditorCamera.OnUpdate(ts);

        m_Framebuffer->Bind();
        Theme theme;
        glClearColor(theme.WindowBg.x, theme.WindowBg.y, theme.WindowBg.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        World* world = Engine::Get().GetWorld();
        if (world) {
            world->OnUpdate(ts, m_EditorCamera.GetViewProjection());
        }
        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnEvent(Event& e)
    {
        if (m_ViewportHovered) m_EditorCamera.OnEvent(e);

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<FileDropEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnFileDrop));
        dispatcher.Dispatch<KeyPressedEvent>(AETHER_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

    bool EditorLayer::OnFileDrop(FileDropEvent& e)
    {
        std::filesystem::path targetDir = m_ContentBrowserPanel.GetCurrentDirectory();
        const auto& paths = e.GetPaths();

        for (const auto& pathString : paths)
        {
            std::filesystem::path sourcePath(pathString);
            if (!std::filesystem::is_regular_file(sourcePath)) continue;

            std::string filename = sourcePath.filename().string();
            std::filesystem::path destinationPath = targetDir / filename;

            try {
                std::filesystem::copy_file(sourcePath, destinationPath, std::filesystem::copy_options::overwrite_existing);
                AssetManager::ImportSourceFile(destinationPath);
                AETHER_CORE_INFO("Dropped and Imported: {}", filename);
            }
            catch (std::filesystem::filesystem_error& ex) {
                AETHER_CORE_ERROR("Import Failed for {}: {}", filename, ex.what());
            }
        }
        return true;
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Safety: Do not trigger shortcuts if user is typing in a text field
        if (ImGui::GetIO().WantTextInput) return false;

        bool control = Input::IsKeyPressed(Key::LeftCtrl) || Input::IsKeyPressed(Key::RightCtrl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode())
        {
        case Key::Z:
        {
            if (control && shift) {
                CommandHistory::Redo();
                return true;
            }
            else if (control) {
                CommandHistory::Undo();
                return true;
            }
            break;
        }
        case Key::Y:
        {
            if (control) {
                CommandHistory::Redo();
                return true;
            }
            break;
        }
        }
        return false;
    }

    void EditorLayer::OnImGuiRender()
    {
        // Ensure blocking stays disabled
        Engine::Get().GetImGuiLayer()->SetBlockEvents(false);

        World* world = Engine::Get().GetWorld();
        if (!world) { ImGui::Text("Loading World..."); return; }

        Theme theme;
        m_SceneHierarchyPanel.SetContext(world->GetScene());
        m_InspectorPanel.SetContext(m_SceneHierarchyPanel.GetSelectedEntity());

        m_ContentBrowserPanel.SetShowRawAssets(m_Settings.ShowRawAssets);

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_MenuBarBg, theme.PanelHover);
        ImGui::Begin("Aether DockSpace", nullptr, window_flags);
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        ImGuiID dockspace_id = ImGui::GetID("AetherMasterDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        if (ImGui::BeginMenuBar())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, theme.AccentPrimary);

            if (ImGui::BeginMenu("File")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Save Project", "Ctrl+S")) {
                    ProjectSerializer s(Project::GetActive());
                    s.Serialize(Project::GetActive()->GetProjectDirectory() / (Project::GetActiveConfig().Name + ".aether"));
                }
                if (ImGui::MenuItem("Exit")) Engine::Get().Close();
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Undo", "Ctrl+Z", false, CommandHistory::CanUndo())) {
                    CommandHistory::Undo();
                }
                if (ImGui::MenuItem("Redo", "Ctrl+Y", false, CommandHistory::CanRedo())) {
                    CommandHistory::Redo();
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Settings")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Preferences...", nullptr, m_ShowPreferences)) {
                    m_ShowPreferences = true;
                }
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                ImGui::PushStyleColor(ImGuiCol_Text, theme.Text);
                if (ImGui::MenuItem("Reset Layout")) EnsureLayout(dockspace_id);
                ImGui::PopStyleColor();
                ImGui::EndMenu();
            }
            ImGui::PopStyleColor();
            ImGui::EndMenuBar();
        }

        if (m_IsFirstFrame) {
            if (!std::filesystem::exists(m_IniFilePath)) EnsureLayout(dockspace_id);
            m_IsFirstFrame = false;
        }

        m_SceneHierarchyPanel.OnImGuiRender();
        m_InspectorPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();

        RenderPreferencesPanel();

        for (auto it = m_AssetEditors.begin(); it != m_AssetEditors.end(); ) {
            (*it)->OnImGuiRender();
            if (!(*it)->IsOpen()) it = m_AssetEditors.erase(it);
            else ++it;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportSize.x, viewportSize.y };

        if (m_Framebuffer)
            ImGui::Image((void*)(uintptr_t)m_Framebuffer->GetColorAttachmentRendererID(), viewportSize, { 0, 1 }, { 1, 0 });

        ImGui::End();
        ImGui::PopStyleVar();

        ImGui::End();
    }

    void EditorLayer::RenderPreferencesPanel()
    {
        if (!m_ShowPreferences) return;

        ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
        ImGui::Begin("Preferences", &m_ShowPreferences, ImGuiWindowFlags_NoCollapse);

        Theme theme;
        ImGui::TextColored(theme.AccentPrimary, "Content Browser");
        ImGui::Separator();

        bool show = m_Settings.ShowRawAssets;
        if (ImGui::Checkbox("Show Raw Source Files", &show))
        {
            m_Settings.ShowRawAssets = show;
            SaveSettings();
        }

        ImGui::SameLine();
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("If enabled, source files (png, jpg) will be visible along with their .aeth assets.\nDisable this to reduce clutter.");
        }

        ImGui::End();
    }

    void EditorLayer::EnsureLayout(unsigned int dockspace_id)
    {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
        ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.20f, nullptr, &dock_main_id);
        ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.30f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow("Viewport", dock_main_id);
        ImGui::DockBuilderDockWindow("Inspector", dock_right_id);
        ImGui::DockBuilderDockWindow("Scene Hierarchy", dock_left_id);
        ImGui::DockBuilderDockWindow("Content Browser", dock_bottom_id);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    void EditorLayer::OpenAsset(const std::filesystem::path& path)
    {
        auto relativePath = std::filesystem::relative(path, Project::GetAssetDirectory());

        for (auto& editor : m_AssetEditors) {
            if (editor->GetAssetPath() == relativePath) {
                editor->SetFocus();
                return;
            }
        }

        if (AssetManager::HasAsset(relativePath)) {
            AssetType type = AssetManager::GetMetadata(relativePath).Type;
            if (type == AssetType::Texture2D) {
                auto panel = std::make_shared<TextureViewerPanel>("Texture Viewer", relativePath);
                m_AssetEditors.push_back(panel);
            }
        }
    }
}
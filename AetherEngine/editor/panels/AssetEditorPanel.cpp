#include "AssetEditorPanel.h"

namespace aether {

    void AssetEditorPanel::OnImGuiRender()
    {
        if (!m_IsOpen) return;

        if (m_FocusRequested)
        {
            ImGui::SetNextWindowFocus();
            m_FocusRequested = false;
        }

        ImGui::SetNextWindowSize(m_DefaultWindowSize, ImGuiCond_FirstUseEver);

        ImGuiWindowFlags flags = ImGuiWindowFlags_MenuBar;
        if (m_IsDirty) flags |= ImGuiWindowFlags_UnsavedDocument;

        std::string id = m_Title + "##" + m_AssetPath.string();

        if (ImGui::Begin(id.c_str(), &m_IsOpen, flags))
        {
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save", "Ctrl+S"))
                        Save();
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }

            // Layout: 2 Columns [Content | Inspector]
            // We use a Table to manage the layout cleanly with resizing support.
            static ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoSavedSettings;

            if (ImGui::BeginTable("EditorLayout", 2, tableFlags))
            {
                // Setup Columns
                // Column 0: Content (Stretches)
                ImGui::TableSetupColumn("View", ImGuiTableColumnFlags_WidthStretch);
                // Column 1: Inspector (Fixed default, but resizable)
                ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthFixed, 200.0f);

                ImGui::TableNextRow();

                // --- Column 0: Content View ---
                ImGui::TableSetColumnIndex(0);
                // Use a Child window to allow the content (e.g., zoomed image) to scroll 
                // independently without affecting the toolbar side.
                ImGui::BeginChild("##ContentRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
                RenderContent();
                ImGui::EndChild();

                // --- Column 1: Inspector ---
                ImGui::TableSetColumnIndex(1);
                ImGui::BeginChild("##InspectorRegion");
                // Add padding for aesthetics
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
                RenderInspector();
                ImGui::PopStyleVar();
                ImGui::EndChild();

                ImGui::EndTable();
            }
        }
        ImGui::End();
    }
}
#include "Theme.h"

namespace aether {

    void ThemeManager::ApplyTheme(const Theme& theme) {
        auto& style = ImGui::GetStyle();
        auto& colors = style.Colors;

        // --- Style Vars ---
        style.WindowRounding = theme.WindowRounding;
        style.FrameRounding = theme.FrameRounding;
        style.GrabRounding = theme.GrabRounding;
        style.PopupRounding = theme.PopupRounding;
        style.ScrollbarRounding = theme.ScrollbarRounding;
        style.TabRounding = theme.TabRounding;
        style.WindowBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f; // Minimalist look

        // --- Colors Mapping ---

        // Backgrounds
        colors[ImGuiCol_WindowBg] = theme.WindowBg;
        colors[ImGuiCol_ChildBg] = theme.PanelBg;
        colors[ImGuiCol_PopupBg] = theme.PanelHover; // Use elevated color for popups

        // Text
        colors[ImGuiCol_Text] = theme.Text;
        colors[ImGuiCol_TextDisabled] = theme.TextMuted;

        // Borders
        colors[ImGuiCol_Border] = theme.Border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

        // Headers (Used for TreeNodes, etc) -> Primary Accent low alpha
        ImVec4 header = theme.AccentPrimary;
        header.w = 0.2f; // Low alpha
        colors[ImGuiCol_Header] = header;

        header.w = 0.4f;
        colors[ImGuiCol_HeaderHovered] = header;

        header.w = 0.6f;
        colors[ImGuiCol_HeaderActive] = header;

        // Buttons -> Panel Background (Minimalist)
        colors[ImGuiCol_Button] = theme.PanelBg;
        colors[ImGuiCol_ButtonHovered] = theme.PanelHover;
        colors[ImGuiCol_ButtonActive] = theme.AccentPrimary; // Pop on click

        // Inputs (Frames)
        colors[ImGuiCol_FrameBg] = theme.WindowBg; // Use darker background for inputs to make them stand out
        colors[ImGuiCol_FrameBgHovered] = theme.PanelHover;
        colors[ImGuiCol_FrameBgActive] = theme.PanelHover;

        // Tabs
        colors[ImGuiCol_Tab] = theme.PanelBg;
        colors[ImGuiCol_TabHovered] = theme.PanelHover;
        colors[ImGuiCol_TabActive] = theme.PanelHover; // Active tab matches panel
        colors[ImGuiCol_TabUnfocused] = theme.PanelBg;
        colors[ImGuiCol_TabUnfocusedActive] = theme.PanelHover;

        // Title Bar
        colors[ImGuiCol_TitleBg] = theme.TitleBar;
        colors[ImGuiCol_TitleBgActive] = theme.TitleBar;
        colors[ImGuiCol_TitleBgCollapsed] = theme.TitleBar;

        // Accents
        colors[ImGuiCol_CheckMark] = theme.AccentCyan;
        colors[ImGuiCol_SliderGrab] = theme.AccentPrimary;
        colors[ImGuiCol_SliderGrabActive] = theme.AccentPrimary;
        colors[ImGuiCol_TextSelectedBg] = theme.AccentSecondary;
        colors[ImGuiCol_DragDropTarget] = theme.AccentCyan;
        colors[ImGuiCol_NavHighlight] = theme.AccentPrimary;
    }
}
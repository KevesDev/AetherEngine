#pragma once
#include <imgui.h>
#include <string>

namespace aether {

    struct Theme {
        std::string Name = "Dark Modern";

        // --- Colors (From our Style Guide) ---

        // Primary background (Very dark blue gray) #0F1217
        ImVec4 WindowBg = ImVec4(0.059f, 0.071f, 0.090f, 1.0f);

        // Panel background (Dark slate blue gray) #161B22
        // Used for cards, panels, containers
        ImVec4 PanelBg = ImVec4(0.086f, 0.106f, 0.133f, 1.0f);

        // Panel elevation (Slightly lighter) #1C212B
        // Used for dropdowns, popups, focused areas
        ImVec4 PanelHover = ImVec4(0.110f, 0.129f, 0.169f, 1.0f);

        // Top bar (Near black) #0B0E13
        ImVec4 TitleBar = ImVec4(0.043f, 0.055f, 0.075f, 1.0f);

        // Borders (Muted dark gray blue) #242A35
        ImVec4 Border = ImVec4(0.141f, 0.165f, 0.208f, 1.0f);

        // Primary Accent (Neon Lavender) #B98CFF
        ImVec4 AccentPrimary = ImVec4(0.725f, 0.549f, 1.0f, 1.0f);

        // Secondary Accent (Deep Violet) #9B6DFF
        ImVec4 AccentSecondary = ImVec4(0.608f, 0.427f, 1.0f, 1.0f);

        // Cyan Accent (Bright Teal) #35D0C6
        ImVec4 AccentCyan = ImVec4(0.208f, 0.816f, 0.776f, 1.0f);

        // Success Accent (Blue Teal) #3CD6C4
        ImVec4 AccentSuccess = ImVec4(0.235f, 0.839f, 0.769f, 1.0f);

        // Text Primary (Off white) #E6E8EC
        ImVec4 Text = ImVec4(0.902f, 0.910f, 0.925f, 1.0f);

        // Text Secondary (Muted cool gray) #9AA1AC
        ImVec4 TextMuted = ImVec4(0.604f, 0.631f, 0.675f, 1.0f);

        // --- Styling ---
        float WindowRounding = 8.0f;
        float FrameRounding = 6.0f;
        float GrabRounding = 6.0f;
        float PopupRounding = 6.0f;
        float ScrollbarRounding = 9.0f;
        float TabRounding = 6.0f;
    };

    class ThemeManager {
    public:
        static void ApplyTheme(const Theme& theme);
    };
}
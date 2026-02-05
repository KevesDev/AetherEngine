#pragma once

#include "../../engine/project/Project.h"

namespace aether {

    class NetworkSettingsPanel {
    public:
        void OnImGuiRender() {
            auto& config = Project::GetActiveConfig();

            ImGui::Text("Network Settings");
            ImGui::Separator();

            int port = static_cast<int>(config.ServerPort);
            if (ImGui::InputInt("Server Port", &port)) {
                if (port > 0 && port <= 65535) {
                    config.ServerPort = static_cast<uint16_t>(port);
                }
            }

            int maxPlayers = static_cast<int>(config.MaxPlayers);
            if (ImGui::InputInt("Max Players", &maxPlayers)) {
                if (maxPlayers > 0 && maxPlayers <= 1024) {
                    config.MaxPlayers = static_cast<uint32_t>(maxPlayers);
                }
            }

            ImGui::Separator();

            ImGui::SliderFloat("Static Replication Rate (Hz)",
                               &config.StaticReplicationRateHz,
                               0.5f, 60.0f);
            ImGui::SliderFloat("Frequent Replication Rate (Hz)",
                               &config.FrequentReplicationRateHz,
                               1.0f, 120.0f);
        }
    };

} // namespace aether


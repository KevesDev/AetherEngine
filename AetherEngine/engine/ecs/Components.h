#pragma once
#include <string>

namespace aether {

    // Defines how this component data is sent over the network
    enum class ReplicationMode {
        None = 0,
        ServerToAll,    // Multicast: Standard for visible objects
        ServerToOwner,  // Private: Inventory, HUD stats, etc.
        InitialOnly     // Optimization: Only send once on spawn
    };

    struct TagComponent {
        std::string Tag;
    };

    struct TransformComponent {
        float X = 0.0f;
        float Y = 0.0f;
        float Rotation = 0.0f;
        float ScaleX = 1.0f;
        float ScaleY = 1.0f;

        ReplicationMode Replication = ReplicationMode::ServerToAll;
    };

    struct SpriteComponent {
        // RGBA Color (0.0f - 1.0f)
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
        float A = 1.0f;

        // TODO: add TextureID here later
    };

}
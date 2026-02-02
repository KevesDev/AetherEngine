#pragma once
#include <string>
#include <vector> 
#include <cstdint> // Added for uint32_t

namespace aether {

    // Define EntityID here to avoid circular dependency with Registry.h
    using EntityID = uint32_t;
    // We match the sentinel value used in Entity.h (-1 cast to uint32_t)
    constexpr EntityID NULL_ENTITY = (EntityID)-1;

    enum class ReplicationMode {
        None = 0,
        ServerToAll,
        ServerToOwner,
        InitialOnly
    };

    struct TagComponent {
        std::string Tag;
    };

    // PRODUCTION STANDARD: The Scene Graph Node
    // Maintains a Doubly-Linked List for O(1) Hierarchy Traversal
    // This allows every entity to know its parent and siblings without pointer chasing.
    struct RelationshipComponent {
        EntityID Parent = NULL_ENTITY;
        EntityID FirstChild = NULL_ENTITY;
        EntityID PreviousSibling = NULL_ENTITY;
        EntityID NextSibling = NULL_ENTITY;

        size_t ChildrenCount = 0;
    };

    struct TransformComponent {
        float X = 0.0f;
        float Y = 0.0f;
        float Rotation = 0.0f;
        float ScaleX = 100.0f;
        float ScaleY = 100.0f;

        ReplicationMode Replication = ReplicationMode::ServerToAll;
    };

    struct SpriteComponent {
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
        float A = 1.0f;
    };

    struct CameraComponent {
        float Size = 720.0f;
        float Near = -1.0f;
        float Far = 1.0f;
        bool Primary = true;
    };
}
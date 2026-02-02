#pragma once
#include <string>
#include <vector> 
#include <cstdint>
#include <glm/glm.hpp> //Required for glm types
#include <glm/gtc/matrix_transform.hpp> // Required for glm::radians

namespace aether {

    using EntityID = uint32_t;
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
        enum class Type { Perspective = 0, Orthographic = 1 };

        Type ProjectionType = Type::Orthographic;

        float PerspectiveFOV = glm::radians(45.0f);
        float PerspectiveNear = 0.01f;
        float PerspectiveFar = 1000.0f;

        float OrthographicSize = 10.0f;
        float OrthographicNear = -1.0f;
        float OrthographicFar = 1.0f;

        bool Primary = true;
        bool FixedAspectRatio = false;
    };
}
#pragma once
#include <string>
#include <vector> 
#include <cstdint>
#include <unordered_map>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

namespace aether {

    using EntityID = uint32_t;
    constexpr EntityID NULL_ENTITY = (EntityID)-1;

    /**
     * Replication modes for authoritative state synchronization.
     */
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

        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), { X, Y, 0.0f })
                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
                * glm::scale(glm::mat4(1.0f), { ScaleX, ScaleY, 1.0f });
        }
    };

    struct SpriteComponent {
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
        float A = 1.0f;
    };

    /**
     * References a developer-defined Logic Graph asset.
     * The Server-side LogicGraphSystem executes this graph during the Simulation stage.
	 * TODO: The graph should be compiled into actual code during the build process for performance.
     */
    struct LogicGraphComponent {
        std::string GraphAssetPath;
        bool IsActive = true;
    };

    /**
     * Links a player to their current input preferences.
     * ActiveMappingContextPath allows for swapping control schemes at runtime.
     */
    struct PlayerControllerComponent {
        uint32_t PlayerIndex = 0;
        std::string ActiveMappingContextPath;
    };

    /**
     * The authoritative container for action values.
     * Logic Graph nodes read from these maps using the ActionID from the Manifest.
     */
    struct InputComponent {
        // Stores Digital (0/1) and Axis1D values
        std::unordered_map<uint32_t, float> ActionStates;

        // Stores Axis2D values (e.g., Mouse Delta or Joystick)
        std::unordered_map<uint32_t, glm::vec2> Axis2DStates;

        float GetActionValue(uint32_t actionID) const {
            auto it = ActionStates.find(actionID);
            return it != ActionStates.end() ? it->second : 0.0f;
        }

        glm::vec2 GetAxis2DValue(uint32_t actionID) const {
            auto it = Axis2DStates.find(actionID);
            return it != Axis2DStates.end() ? it->second : glm::vec2(0.0f);
        }
    };
}
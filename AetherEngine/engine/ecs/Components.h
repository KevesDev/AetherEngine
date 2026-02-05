#pragma once

// Enable GLM experimental extensions BEFORE including GLM headers
#define GLM_ENABLE_EXPERIMENTAL

#include <string>
#include <vector> 
#include <array>
#include <cstdint>

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>

#include "../core/UUID.h"
#include "../renderer/CameraUtils.h"

namespace aether {

    using EntityID = uint32_t;
    constexpr EntityID NULL_ENTITY = (EntityID)-1;

    // --- Core Components ---

    struct IDComponent {
        UUID ID;

        IDComponent() = default;
        IDComponent(const UUID& uuid) : ID(uuid) {}
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
        float ScaleX = 1.0f;
        float ScaleY = 1.0f;

        float PrevX = 0.0f;
        float PrevY = 0.0f;
        float PrevRotation = 0.0f;

        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), { X, Y, 0.0f })
                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
                * glm::scale(glm::mat4(1.0f), { ScaleX, ScaleY, 1.0f });
        }

        glm::mat4 GetInterpolatedTransform(float alpha) const {
            float iX = PrevX + (X - PrevX) * alpha;
            float iY = PrevY + (Y - PrevY) * alpha;
            float iRot = PrevRotation + (Rotation - PrevRotation) * alpha;

            return glm::translate(glm::mat4(1.0f), { iX, iY, 0.0f })
                * glm::rotate(glm::mat4(1.0f), glm::radians(iRot), { 0.0f, 0.0f, 1.0f })
                * glm::scale(glm::mat4(1.0f), { ScaleX, ScaleY, 1.0f });
        }
    };

    struct SpriteComponent {
        float R = 1.0f;
        float G = 1.0f;
        float B = 1.0f;
        float A = 1.0f;
    };

    // --- Camera Component ---
    struct CameraComponent {
        enum class Type { Perspective = 0, Orthographic = 1 };

        Type ProjectionType = Type::Orthographic;

        float PerspFOV = glm::radians(45.0f);
        float PerspNear = 0.01f;
        float PerspFar = 1000.0f;

        float OrthoSize = 10.0f;
        float OrthoNear = -1.0f;
        float OrthoFar = 1.0f;

        float AspectRatio = 1.778f;

        bool Primary = true;
        bool FixedAspectRatio = false;

        // Runtime calculated projection
        glm::mat4 GetProjection() const {
            if (ProjectionType == Type::Perspective)
                return CameraUtils::CalculatePerspective(PerspFOV, AspectRatio, PerspNear, PerspFar);
            else
                return CameraUtils::CalculateOrthographic(OrthoSize, AspectRatio, OrthoNear, OrthoFar);
        }

        // Legacy compatibility
        // TODO: Probably don't need but check later
        float OrthographicSize = 10.0f;
        float OrthographicNear = -1.0f;
        float OrthographicFar = 1.0f;
        float PerspectiveFOV = glm::radians(45.0f);
        float PerspectiveNear = 0.01f;
        float PerspectiveFar = 1000.0f;
    };

    // --- Gameplay & Input Components ---

    struct LogicGraphComponent {
        uint64_t GraphAssetHandle = 0;
        bool IsActive = true;
    };

    struct PlayerControllerComponent {
        uint32_t PlayerIndex = 0;
        uint64_t ActiveMappingContext = 0;
    };

    struct InputFrame {
        uint32_t Tick = 0;
        uint32_t ActionCount = 0;

        struct ActionState {
            uint32_t ActionID;
            float Value;
        };

        static constexpr size_t MAX_ACTIONS_PER_FRAME = 32;
        std::array<ActionState, MAX_ACTIONS_PER_FRAME> Actions;

        void SetAction(uint32_t actionID, float value) {
            for (size_t i = 0; i < ActionCount; ++i) {
                if (Actions[i].ActionID == actionID) {
                    Actions[i].Value = value;
                    return;
                }
            }
            if (ActionCount < MAX_ACTIONS_PER_FRAME) {
                Actions[ActionCount] = { actionID, value };
                ActionCount++;
            }
        }

        float GetAction(uint32_t actionID) const {
            for (size_t i = 0; i < ActionCount; ++i) {
                if (Actions[i].ActionID == actionID) return Actions[i].Value;
            }
            return 0.0f;
        }

        void Clear() { ActionCount = 0; }
    };

    constexpr size_t INPUT_BUFFER_SIZE = 64;

    struct InputComponent {
        std::array<InputFrame, INPUT_BUFFER_SIZE> InputHistory;
        uint32_t CurrentTick = 0;

        void SetAction(uint32_t actionID, float value) {
            InputFrame& frame = InputHistory[CurrentTick % INPUT_BUFFER_SIZE];
            if (frame.Tick != CurrentTick) {
                frame.Tick = CurrentTick;
                frame.Clear();
            }
            frame.SetAction(actionID, value);
        }

        float GetActionValue(uint32_t actionID) const {
            const InputFrame& frame = InputHistory[CurrentTick % INPUT_BUFFER_SIZE];
            if (frame.Tick != CurrentTick) return 0.0f;
            return frame.GetAction(actionID);
        }
    };

    // Marks an entity as eligible for network replication and carries
    // configuration for how and when it should be replicated.
    enum class ReplicationMode : std::uint8_t {
        None = 0,
        Static = 1,      // Replicate on creation / rare updates only
        Frequent = 2     // Replicate frequently (e.g. characters, projectiles)
    };

    struct ReplicationComponent {
        ReplicationMode Mode = ReplicationMode::Frequent;
        float UpdateRateHz = 20.0f;  // Target replication frequency
        float Accumulator = 0.0f;    // Internal accumulator used by replication system
    };
}
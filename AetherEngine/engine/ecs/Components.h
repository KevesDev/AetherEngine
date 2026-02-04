#pragma once

#include <string>
#include <vector> 
#include <array>
#include <cstdint>

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtx/quaternion.hpp>

#include "../core/UUID.h"

namespace aether {

    using EntityID = uint32_t;
    constexpr EntityID NULL_ENTITY = (EntityID)-1;

    // --- Core Components ---

    /**
     * IDComponent
     * * Defines the stable, unique identifier for an entity.
     * * IMPORTANT:
     * Unlike runtime EntityIDs (which change every run), UUIDs persist
     * across saves and network replication. Essential for the Serializer.
     */
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
        // Current Simulation State (The Truth)
        float X = 0.0f;
        float Y = 0.0f;
        float Rotation = 0.0f;
        float ScaleX = 1.0f;
        float ScaleY = 1.0f;

        // Previous Simulation State (For Interpolation)
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

    // --- Gameplay & Input Components ---

    struct LogicGraphComponent {
        uint64_t GraphAssetHandle = 0;
        bool IsActive = true;
    };

    struct PlayerControllerComponent {
        uint32_t PlayerIndex = 0;
        uint64_t ActiveMappingContext = 0; // AssetHandle
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
}
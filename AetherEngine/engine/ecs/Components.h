#pragma once
#include <string>
#include <vector> 
#include <array>
#include <cstdint>
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 

namespace aether {

    using EntityID = uint32_t;
    constexpr EntityID NULL_ENTITY = (EntityID)-1;

    // --- Core Components ---

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

    /**
     * TransformComponent: Represents the spatial state of an entity.
     * Includes 'Previous' state fields to support fixed-step interpolation.
     * This allows the Rendering system to smooth movement between 60Hz simulation ticks.
     */
    struct TransformComponent {
        // Current Simulation State (The Truth)
        float X = 0.0f;
        float Y = 0.0f;
        float Rotation = 0.0f;
        float ScaleX = 1.0f;
        float ScaleY = 1.0f;

        // Previous Simulation State (For Interpolation)
        // These are updated automatically by the Physics/Simulation System at the start of a tick.
        float PrevX = 0.0f;
        float PrevY = 0.0f;
        float PrevRotation = 0.0f;

        // Helper to get the Current Transform Matrix
        glm::mat4 GetTransform() const {
            return glm::translate(glm::mat4(1.0f), { X, Y, 0.0f })
                * glm::rotate(glm::mat4(1.0f), glm::radians(Rotation), { 0.0f, 0.0f, 1.0f })
                * glm::scale(glm::mat4(1.0f), { ScaleX, ScaleY, 1.0f });
        }

        // Helper to get the Interpolated Transform Matrix (for Rendering)
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
        std::string GraphAssetPath;
        bool IsActive = true;
    };

    struct PlayerControllerComponent {
        uint32_t PlayerIndex = 0;
        std::string ActiveMappingContextPath;
    };

    /**
     * InputFrame: Represents the input state for a single simulation tick.
     * Uses fixed-size arrays to ensure the struct is Trivially Copyable (POD).
     * This is required for O(1) memcpy during rollback/prediction operations.
     */
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
            // Update existing
            for (size_t i = 0; i < ActionCount; ++i) {
                if (Actions[i].ActionID == actionID) {
                    Actions[i].Value = value;
                    return;
                }
            }
            // Add new if space permits
            if (ActionCount < MAX_ACTIONS_PER_FRAME) {
                Actions[ActionCount] = { actionID, value };
                ActionCount++;
            }
        }

        float GetAction(uint32_t actionID) const {
            for (size_t i = 0; i < ActionCount; ++i) {
                if (Actions[i].ActionID == actionID) {
                    return Actions[i].Value;
                }
            }
            return 0.0f;
        }

        void Clear() {
            ActionCount = 0;
        }
    };

    constexpr size_t INPUT_BUFFER_SIZE = 64;

    struct InputComponent {
        // Contiguous memory ring buffer for input history
        std::array<InputFrame, INPUT_BUFFER_SIZE> InputHistory;
        uint32_t CurrentTick = 0;

        void SetAction(uint32_t actionID, float value) {
            InputFrame& frame = InputHistory[CurrentTick % INPUT_BUFFER_SIZE];

            // Frame Initialization check
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
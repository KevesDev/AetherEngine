#pragma once

#include "System.h"
#include "../Engine.h"
#include "../../ecs/Components.h"
#include "../../ecs/Registry.h"
#include "../../input/Input.h"
#include "../../input/InputMappingContext.h"
#include "../../asset/AssetManager.h"

#include <unordered_map>

namespace aether {

    /**
     * InputSystem
     * The bridge between Hardware Input and the Game Simulation.
     * * IMPORTANT ARCHITECTURE:
     * 1. Hardware Poll: Checks physical key states (keyboard/mouse/gamepad).
     * 2. Context Resolution: Maps physical keys to logical Action IDs via AssetManager.
     * 3. Accumulation: Sums inputs (e.g., W=+1, S=-1 -> Result=0) to prevent masking.
     * 4. Ring Buffer Commit: Writes the final frame to the InputComponent for the current tick.
     */
    class InputSystem : public ISystem {
    public:
		// Get system name
        const char* GetName() const override { return "InputSystem"; }

        void OnUpdate(Registry& reg, float deltaTime) override {
            // Server Safety: The Server has no physical input hardware.
            // Input on the server is driven by the NetworkSystem (packet ingestion), not this system.
            if (Engine::Get().GetAppType() == ApplicationType::Server) return;

            // Use standard Registry view syntax
            auto view = reg.view<PlayerControllerComponent>();
            for (auto entity : view) {
                // Retrieve component via Engine API (GetComponent)
                const auto* controller = reg.GetComponent<PlayerControllerComponent>(entity);

                // 1. Validate Context
                if (!controller || controller->ActiveMappingContext == 0) continue;

                auto context = AssetManager::GetAsset<InputMappingContext>(controller->ActiveMappingContext);
                if (!context) continue;

                // 2. Prepare Input Component
                // Use Engine API HasComponent/AddComponent
                if (!reg.HasComponent<InputComponent>(entity)) {
                    reg.AddComponent<InputComponent>(entity, InputComponent{});
                }
                auto* inputComp = reg.GetComponent<InputComponent>(entity);

                // 3. Frame Initialization
                // We must clear the actions for the current tick before accumulating.
                // This prevents "stuck" inputs from previous frames if keys are released.
                inputComp->CurrentTick++; // Advance simulation tick (Client Authoritative step)

                // Get reference to the frame we are about to write
                // Accessing the ring buffer manually here to clear it effectively
                InputFrame& currentFrame = inputComp->InputHistory[inputComp->CurrentTick % INPUT_BUFFER_SIZE];
                currentFrame.Tick = inputComp->CurrentTick;
                currentFrame.Clear();

                // 4. Input Accumulation
                // We use a local map to sum values. This handles conflicting inputs 
                // (e.g. Left + Right) correctly by cancelling them out before writing.
                std::unordered_map<uint32_t, float> frameValues;

                for (const auto& mapping : context->GetMappings()) {
                    // Poll Hardware
                    // TODO: Abstraction for Axis/Gamepad support would happen here.
                    // Currently maps digital key press to float value.
                    if (Input::IsKeyPressed(mapping.KeyCode)) {
                        // Accumulate: Add scale (e.g. 1.0 or -1.0)
                        frameValues[mapping.ActionID] += mapping.Scale;
                    }
                }

                // 5. Commit to Ring Buffer
                for (const auto& [actionID, value] : frameValues) {
                    // Only write non-zero values to save bandwidth/storage
                    // Logic allows value to exceed 1.0 (e.g. analog triggers + modifiers), 
                    // clamping should happen in the gameplay logic if desired.
                    if (value != 0.0f) {
                        currentFrame.SetAction(actionID, value);
                    }
                }
            }
        }
    };
}
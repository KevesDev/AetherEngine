#pragma once
#include "System.h"
#include "../../ecs/Components.h"
#include "../../input/Input.h"
#include "../../input/InputMappingContext.h"

namespace aether {

    /**
     * InputSystem: A generic, data-driven bridge between hardware and ECS.
     * Translates physical states into logical data based on the provided IMC asset.
     */
    class InputSystem : public ISystem {
    public:
        InputSystem() = default;

        /**
         * Performs the input-to-action translation.
         * The active context must be explicitly set by the engine or player controller state.
         */
        virtual void OnUpdate(Registry& reg, float ts) override {
            if (!m_ActiveContext) return;

            auto view = reg.view<PlayerControllerComponent, InputComponent>();
            for (auto entityID : view) {
                auto& inputComp = reg.GetComponent<InputComponent>(entityID);

                for (const auto& mapping : m_ActiveContext->GetMappings()) {
                    if (Input::IsKeyPressed(mapping.KeyCode)) {
                        inputComp.ActionStates[mapping.ActionID] = 1.0f;
                    }
                }
            }
        }

        void SetActiveContext(InputMappingContext* context) { m_ActiveContext = context; }
        virtual const char* GetName() const override { return "InputSystem"; }

    private:
        InputMappingContext* m_ActiveContext = nullptr;
    };

}
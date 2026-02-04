#pragma once
#include "System.h"
#include "../../ecs/Components.h"
#include "../../input/Input.h"
#include "../../input/InputMappingContext.h"
#include "../Engine.h"
#include "../AetherTime.h"
#include <unordered_map>
#include <memory>

namespace aether {

    class InputSystem : public ISystem {
    public:
        InputSystem() = default;

        virtual void OnUpdate(Registry& reg, float ts) override {
            // Authoritative: Server receives inputs via network, not SDL polling.
            if (Engine::Get().GetAppType() == ApplicationType::Server) return;

            // In a fixed-step loop, we use the global simulation tick
            uint32_t currentTick = (uint32_t)(AetherTime::GetTime() / ts);

            auto view = reg.view<PlayerControllerComponent, InputComponent>();
            for (auto entityID : view) {
                auto& controller = reg.GetComponent<PlayerControllerComponent>(entityID);
                auto& inputComp = reg.GetComponent<InputComponent>(entityID);

                // Update Ring Buffer Head
                inputComp.CurrentTick = currentTick;

                if (controller.ActiveMappingContextPath.empty()) continue;

                // Asset Resolution
                std::shared_ptr<InputMappingContext> context = GetContext(controller.ActiveMappingContextPath);
                if (!context) continue;

                // Map Hardware -> Logical Ring Buffer
                // We do not 'clear' the buffer; we simply write to the slot for 'currentTick'
                // inside SetAction, effectively overwriting old data if the buffer wraps.
                for (const auto& mapping : context->GetMappings()) {
                    if (Input::IsKeyPressed(mapping.KeyCode)) {
                        // Accumulate scale (simple addition for same-frame inputs)
                        float currentValue = inputComp.GetActionValue(mapping.ActionID);
                        inputComp.SetAction(mapping.ActionID, currentValue + mapping.Scale);
                    }
                }
            }
        }

        virtual const char* GetName() const override { return "InputSystem"; }

    private:
        std::shared_ptr<InputMappingContext> GetContext(const std::string& path) {
            auto it = m_ContextCache.find(path);
            if (it != m_ContextCache.end()) return it->second;

            // Load from disk using the implemented JSON loader
            auto newContext = InputMappingContext::Load(path);
            if (newContext) m_ContextCache[path] = newContext;

            return newContext;
        }

        std::unordered_map<std::string, std::shared_ptr<InputMappingContext>> m_ContextCache;
    };
}
#pragma once
#include "../System.h"
#include "../../ecs/Components.h"
#include "../../input/Input.h"
#include <map>

namespace aether {

    // A mapping of logical Action Names to physical KeyCodes
    using ActionMap = std::map<std::string, int>;

    /**
     * InputSystem translates physical hardware state into logical InputComponent data.
     */
    class InputSystem : public ISystem {
    public:
        InputSystem(const ActionMap& map) : m_ActionMap(map) {}

        virtual void OnUpdate(Registry& reg, float ts) override {
            auto view = reg.view<PlayerControllerComponent, InputComponent>();

            for (auto entityID : view) {
                auto* inputComp = reg.GetComponent<InputComponent>(entityID);

                // Iterate through the map to translate hardware state to logical state
                for (auto const& [actionName, keyCode] : m_ActionMap) {
                    inputComp->Actions[actionName] = Input::IsKeyPressed(keyCode);
                }
            }
        }

        virtual const char* GetName() const override { return "InputSystem"; }

    private:
        ActionMap m_ActionMap;
    };

}
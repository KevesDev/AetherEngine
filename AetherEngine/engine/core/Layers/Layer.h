#pragma once

#include "../events/Event.h"
#include <string>

namespace aether {

	/* Abstract base class for layers. For example:
    *  Overlays (Top): Debug Console, Editor UI. They get input events first.
    *  Layers (Bottom): Game World, Player Controller. They get input events last.
    */
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        // Called when the layer is added to the stack
        virtual void OnAttach() {}
        // Called when the layer is removed
        virtual void OnDetach() {}

        // Called every frame (for Game Logic)
        virtual void OnUpdate() {}

        // Called when an event occurs (for Input)
        virtual void OnEvent(Event& event) {}

		// Get the layer's debug name
        inline const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

}
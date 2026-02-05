#pragma once

#include "../AetherTime.h"
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

        // Called when the layer is added/removed to the stack
        virtual void OnAttach() {}
        virtual void OnDetach() {}

        // Variable-step per-frame update driven by the frame clock.
        // Intended for editor/client behavior, camera movement, and other
        // non-authoritative logic that can depend on real frame time.
        // Authoritative gameplay simulation must run inside Scene::OnUpdateSimulation
        // via the SystemScheduler using the fixed simulation timestep.
        virtual void OnUpdate(TimeStep ts) {}

        // Called every frame for ImGui rendering (for UI)
        virtual void OnImGuiRender() {}

        // Legacy no-argument update. Prefer the TimeStep overload.
        virtual void OnUpdate() {}

        // Called when an event occurs (for Input)
        virtual void OnEvent(Event& event) {}

		// Get the layer's debug name
        inline const std::string& GetName() const { return m_DebugName; }
    protected:
        std::string m_DebugName;
    };

}
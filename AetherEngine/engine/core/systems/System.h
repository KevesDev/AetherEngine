#pragma once
#pragma once
#include "../ecs/Registry.h"

namespace aether {

    /**
     * SystemGroup defines the rigid execution order of the engine simulation.
     * This ensures producers always run before consumers.
     */
    enum class SystemGroup {
        Input,      // Gathering raw input and translating to Action Mappings
        Simulation, // Physics, AI, and Gameplay Logic
        Sync,       // State replication and network synchronization
        Render      // Visual updates (Typically skipped by Headless Servers)
    };

    class ISystem {
    public:
        virtual ~ISystem() = default;

        /**
         * @param reg The Registry to operate on
         * @param ts The timestep (Fixed for Simulation/Sync, Variable for Render)
         */
        virtual void OnUpdate(Registry& reg, float ts) = 0;

        virtual const char* GetName() const = 0;
    };

}
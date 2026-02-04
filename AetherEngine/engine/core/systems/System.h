#pragma once
#include "../../ecs/Registry.h"

namespace aether {

    /**
     * SystemGroup defines the rigid execution order of the engine simulation.
     * This ensures producers (Input) always run before consumers (Simulation/Render).
     */
    enum class SystemGroup {
        Input,      // Translating hardware intents to logical Action states
        Simulation, // Authoritative Logic Graph execution (Server-side)
        Sync,       // Network state replication and world synchronization
        Render      // Visual translation and interpolation (Client-side)
    };

    /**
     * ISystem: The pure interface for all engine-level logic.
     */
    class ISystem {
    public:
        virtual ~ISystem() = default;

        /**
         * @param reg The active Registry containing entity data.
         * @param ts The simulation timestep (Fixed for Sim/Sync, Variable for Render).
         */
        virtual void OnUpdate(Registry& reg, float ts) = 0;

        virtual const char* GetName() const = 0;
    };

}
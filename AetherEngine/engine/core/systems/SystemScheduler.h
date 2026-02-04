#pragma once
#include "System.h"
#include <vector>
#include <memory>
#include <array>

namespace aether {

    /**
     * SystemScheduler
     * Orchestrates the execution of systems in a guaranteed order.
     * * ARCHITECTURE NOTE:
     * This class uses a Fixed-Step Accumulator for deterministic 60Hz simulation.
     * Storage is optimized using std::array for O(1) group lookup during the hot loop.
     */
    class SystemScheduler {
    public:
        SystemScheduler() = default;

        // -------------------------------------------------------------------------
        // Core API: Runtime Injection
        // -------------------------------------------------------------------------

        /**
         * Takes ownership of a pre-created system and schedules it.
         * Primary path for Data-Driven systems (e.g., created via SystemRegistry from scene files).
         */
        void AddSystem(SystemGroup group, std::unique_ptr<ISystem> system) {
            if (system) {
                size_t index = static_cast<size_t>(group);
                if (index < GroupCount) {
                    m_Systems[index].push_back(std::move(system));
                }
            }
        }


        // -------------------------------------------------------------------------
        // Execution Pipeline
        // -------------------------------------------------------------------------

        void Update(Registry& reg, float variableDeltaTime) {
            // 1. Variable Stage: Gather hardware/network input intents
            // Must run every frame to prevent input lag.
            RunGroup(SystemGroup::Input, reg, variableDeltaTime);

            // 2. Fixed-Step Accumulation (Deterministic Logic)
            // Simulates physics/gameplay at a locked 60Hz regardless of framerate.
            m_Accumulator += variableDeltaTime;
            while (m_Accumulator >= m_FixedTimeStep) {
                RunGroup(SystemGroup::Simulation, reg, m_FixedTimeStep);
                RunGroup(SystemGroup::Sync, reg, m_FixedTimeStep);
                m_Accumulator -= m_FixedTimeStep;
            }

            // 3. Variable Stage: Rendering and Interpolation
            // Interpolates state between fixed steps for smooth visuals.
            RunGroup(SystemGroup::Render, reg, variableDeltaTime);
        }

        void SetFixedTimeStep(float seconds) { m_FixedTimeStep = seconds; }

    private:
        void RunGroup(SystemGroup group, Registry& reg, float ts) {
            // Direct array access (O(1)) avoids the overhead of map lookups in the hot loop.
            size_t index = static_cast<size_t>(group);
            for (auto& system : m_Systems[index]) {
                system->OnUpdate(reg, ts);
            }
        }

    private:
        // SystemGroup has 4 enum values. We use std::array for cache locality and speed.
        static constexpr size_t GroupCount = 4;
        std::array<std::vector<std::unique_ptr<ISystem>>, GroupCount> m_Systems;

        float m_Accumulator = 0.0f;
        float m_FixedTimeStep = 1.0f / 60.0f;
    };
}
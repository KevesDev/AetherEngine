#pragma once
#include "System.h"
#include <vector>
#include <memory>
#include <map>

namespace aether {

    /*
     * Orchestrates the execution of systems in a guaranteed order.
     * Implements a Fixed-Step Accumulator for deterministic 60Hz simulation.
     */

    class SystemScheduler {
    public:
        SystemScheduler() = default;

        template<typename T, typename... Args>
        void AddSystem(SystemGroup group, Args&&... args) {
            m_Systems[group].push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /**
         * Drives the fixed-step simulation clock.
         * Ensures that Simulation and Sync stages run at a constant frequency.
         */
        void Update(Registry& reg, float variableDeltaTime) {
            // 1. Variable Stage: Gather hardware/network input intents
            RunGroup(SystemGroup::Input, reg, variableDeltaTime);

            // 2. Fixed-Step Accumulation (Deterministic Logic)
            m_Accumulator += variableDeltaTime;
            while (m_Accumulator >= m_FixedTimeStep) {
                RunGroup(SystemGroup::Simulation, reg, m_FixedTimeStep);
                RunGroup(SystemGroup::Sync, reg, m_FixedTimeStep);
                m_Accumulator -= m_FixedTimeStep;
            }

            // 3. Variable Stage: Rendering and Interpolation
            RunGroup(SystemGroup::Render, reg, variableDeltaTime);
        }

        void SetFixedTimeStep(float seconds) { m_FixedTimeStep = seconds; }

    private:
        void RunGroup(SystemGroup group, Registry& reg, float ts) {
            auto it = m_Systems.find(group);
            if (it != m_Systems.end()) {
                for (auto& system : it->second) {
                    system->OnUpdate(reg, ts);
                }
            }
        }

    private:
        std::map<SystemGroup, std::vector<std::unique_ptr<ISystem>>> m_Systems;
        float m_Accumulator = 0.0f;
        float m_FixedTimeStep = 1.0f / 60.0f;
    };

}
#pragma once
#include "System.h"
#include <vector>
#include <memory>
#include <map>

namespace aether {

    class SystemScheduler {
    public:
        SystemScheduler() = default;

        /**
         * Adds a system to a specific execution group.
         * Systems within the same group are executed in the order they are added.
         */
        template<typename T, typename... Args>
        void AddSystem(SystemGroup group, Args&&... args) {
            m_Systems[group].push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }

        /**
         * Updates the simulation using a fixed timestep.
         * Crucial for deterministic behavior on Headless Servers.
         */
        void Update(Registry& reg, float variableDeltaTime) {
            // 1. Variable Stage: Input (Always catch the latest raw input)
            RunGroup(SystemGroup::Input, reg, variableDeltaTime);

            // 2. Fixed-Step Accumulation
            m_Accumulator += variableDeltaTime;

            while (m_Accumulator >= m_FixedTimeStep) {
                // Run Simulation and Sync at a constant frequency (e.g., 60Hz)
                RunGroup(SystemGroup::Simulation, reg, m_FixedTimeStep);
                RunGroup(SystemGroup::Sync, reg, m_FixedTimeStep);

                m_Accumulator -= m_FixedTimeStep;
            }

            // 3. Variable Stage: Render (Interpolation happens here for high refresh monitors)
            RunGroup(SystemGroup::Render, reg, variableDeltaTime);
        }

        void SetFixedTimeStep(float seconds) { m_FixedTimeStep = seconds; }
        float GetFixedTimeStep() const { return m_FixedTimeStep; }

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
        float m_FixedTimeStep = 1.0f / 60.0f; // Default 60Hz simulation
    };

}
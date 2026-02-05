#include "AetherTime.h"
#include <chrono>
#include <SDL.h>

namespace aether {
    using clock = std::chrono::high_resolution_clock;

    // -------------------------------------------------------------------------
    // Internal State
    // -------------------------------------------------------------------------

    // Frame clock
    static clock::time_point s_LastFrameTime;
    static double s_FrameDeltaSeconds = 0.0;

    // Simulation clock
    static double     s_FixedTimeStepSeconds = 1.0 / 60.0; // Default 60 Hz
    static std::uint64_t s_SimTick = 0;

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------

    void AetherTime::Init() {
        s_LastFrameTime = clock::now();
        s_FrameDeltaSeconds = 0.0;
        s_SimTick = 0;
    }

    void AetherTime::UpdateFrame() {
        auto now = clock::now();
        s_FrameDeltaSeconds = std::chrono::duration<double>(now - s_LastFrameTime).count();
        s_LastFrameTime = now;
    }

    double AetherTime::GetFrameDelta() {
        return s_FrameDeltaSeconds;
    }

    double AetherTime::GetFixedTimeStep() {
        return s_FixedTimeStepSeconds;
    }

    void AetherTime::SetFixedTimeStep(double stepSeconds) {
        if (stepSeconds > 0.0) {
            s_FixedTimeStepSeconds = stepSeconds;
        }
    }

    std::uint64_t AetherTime::GetSimTick() {
        return s_SimTick;
    }

    void AetherTime::AdvanceSimulationTick() {
        ++s_SimTick;
    }
}
#pragma once

namespace aether {
    // Defines a span of time (delta time) with helper conversions.
    class TimeStep
    {
    public:
        TimeStep(float time = 0.0f)
            : m_Time(time)
        {
        }

        // Conversions
        float GetSeconds() const { return m_Time; }
        float GetMilliseconds() const { return m_Time * 1000.0f; }

        // Allow implicit usage as a float (e.g., position += speed * ts;)
        operator float() const { return m_Time; }

    private:
        float m_Time;
    };

    /**
     * AetherTime
     *
     * Centralized time source for the engine.
     * - Frame clock: variable real time between frames (used for rendering/UI/editor).
     * - Simulation clock: fixed-step tick index and timestep (used for deterministic gameplay/physics).
     */
    class AetherTime {
    public:
        // Initialize the time system for the engine.
        static void Init();

        // ---------------------------------------------------------------------
        // Frame Clock (Real Time)
        // ---------------------------------------------------------------------

        // Updates the frame clock and computes delta between the current and last frame.
        // Called once per frame by the Engine main loop.
        static void UpdateFrame();

        // Frame delta in seconds (variable). Intended for rendering/UI/editor only.
        static double GetFrameDelta();

        // ---------------------------------------------------------------------
        // Simulation Clock (Fixed-Step)
        // ---------------------------------------------------------------------

        // Global fixed timestep in seconds. Used by deterministic simulation.
        static double GetFixedTimeStep();
        static void SetFixedTimeStep(double stepSeconds);

        // Monotonically increasing simulation tick index.
        static std::uint64_t GetSimTick();

        // Called once per completed fixed-step simulation tick.
        // Typically invoked by the SystemScheduler after running Simulation/Sync groups.
        static void AdvanceSimulationTick();
    };
}
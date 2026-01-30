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

	class AetherTime {
	public:

		// Initialize the time system for the engine.
		static void Init();
		static void Update();

		// One source of truth for delta time in seconds.
		static double DeltaTime();
	};
}
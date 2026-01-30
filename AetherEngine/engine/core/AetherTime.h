#pragma once

namespace aether {
	class AetherTime {
	public:

		// Initialize the time system for the engine.
		static void Init();
		static void Update();

		// One source of truth for delta time in seconds.
		static double DeltaTime();
	};
}
#include "AetherTime.h"
#include <chrono>
#include <SDL.h>

namespace aether {
	using clock = std::chrono::high_resolution_clock;

	// Sets the lastTime to the current time.
	static clock::time_point lastTime;
	static double deltaTime = 0.0;

	// Update the delta time based on the current time.
	void AetherTime::Init() {
		lastTime = clock::now();
		deltaTime = 0.0;
	}

	// This is the time elapsed between frames.
	void AetherTime::Update() {
		auto now = clock::now();
		deltaTime = std::chrono::duration<double>(now - lastTime).count();
		lastTime = now;
	}

	// Get the delta time in seconds.
	double AetherTime::DeltaTime() {
		return deltaTime;
	}

}
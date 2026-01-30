#include "Engine.h"
#include "Log.h"
#include "AetherTime.h"

namespace aether
{
	aether::Engine::Engine()
	{
		aether::Log::Write(aether::LogLevel::Info, "Aether Engine initialized.");
	}

	// Start the engine loop
	void aether::Engine::Run()
	{
		aether::Log::Write(aether::LogLevel::Info, "Aether Engine starting.");
		AetherTime::Init();


		bool running = true;
		int tickCount = 0;

		// Main loop
		while (running) {
			AetherTime::Update();

			Log::Write(LogLevel::Info,
				"Frame dt = " + std::to_string(AetherTime::DeltaTime()));

			tickCount++;
			if (tickCount >= 5) {
				Log::Write(LogLevel::Info, "Engine stopping.");
				running = false;
			}
		}
	}

}
#pragma once
#include <string>

namespace aether
{
	enum class LogLevel
	{
		Debug,
		Info,
		Warning,
		Error,
		Critical
	};

	// Simple logging class for the Aether Engine.
	class Log
	{
	public:
		static void Write(LogLevel level, const std::string& message);
	};
};
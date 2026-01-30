#include "Log.h"
#include <iostream>

namespace aether
{
	void aether::Log::Write(LogLevel level, const std::string& message)
	{
		switch (level)
		{
			case aether::LogLevel::Debug:
				std::cout << "[DEBUG]: ";
				break;
			case aether::LogLevel::Info:
				std::cout << "[INFO]: ";
				break;
			case aether::LogLevel::Warning:
				std::cout << "[WARNING]: ";
				break;
			case aether::LogLevel::Error:
				std::cout << "[ERROR]: ";
				break;
			case aether::LogLevel::Critical:
				std::cout << "[CRITICAL]: ";
				break;
			default:
				break;
		}
		std::cout << message << std::endl;
	}
}

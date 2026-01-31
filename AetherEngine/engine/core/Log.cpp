#include "Log.h"

namespace aether
{
    void Log::Write(LogLevel level, const std::string& message)
    {
        switch (level)
        {
            // ANSI Escape Codes for colored output (supported by most modern terminals, including VS Code and newer VS)
        case LogLevel::Debug:    std::cout << "\033[37m[DEBUG]: \033[0m"; break; // Gray
        case LogLevel::Info:     std::cout << "\033[32m[INFO]:  \033[0m"; break; // Green
        case LogLevel::Warning:  std::cout << "\033[33m[WARN]:  \033[0m"; break; // Yellow
        case LogLevel::Error:    std::cout << "\033[31m[ERROR]: \033[0m"; break; // Red
        case LogLevel::Critical: std::cout << "\033[41m[CRIT]:  \033[0m"; break; // Red Background
        }
        std::cout << message << std::endl;
    }
}
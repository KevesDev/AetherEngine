#pragma once
#include <string>
#include <format>
#include <vector>
#include <mutex>
#include <fstream>

// --- Platform Detection & Breakpoint Macro ---
#if defined(_MSC_VER)
#define AETHER_DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define AETHER_DEBUGBREAK() __builtin_trap()
#else
#include <cstdlib>
#define AETHER_DEBUGBREAK() std::abort()
#endif

namespace aether
{
    enum class LogLevel { Debug, Info, Warning, Error, Critical };

    struct LogEntry {
        LogLevel Level;
        std::string Message;
        std::string Timestamp;
    };

    class Log
    {
    public:
        static void Init();
        static void Write(LogLevel level, const std::string& message);

        static const std::vector<LogEntry>& GetHistory() { return s_LogHistory; }

        // Using std::format_string ensures compile-time check of your log arguments
        template<typename... Args>
        static void Print(LogLevel level, std::format_string<Args...> fmt, Args&&... args) {
            Write(level, std::format(fmt, std::forward<Args>(args)...));
        }

    private:
        static std::vector<LogEntry> s_LogHistory;
        static std::ofstream s_LogFile;
        static std::mutex s_LogMutex;
    };
}

#define AETHER_CORE_TRACE(...)    ::aether::Log::Print(::aether::LogLevel::Debug, __VA_ARGS__)
#define AETHER_CORE_INFO(...)     ::aether::Log::Print(::aether::LogLevel::Info, __VA_ARGS__)
#define AETHER_CORE_WARN(...)     ::aether::Log::Print(::aether::LogLevel::Warning, __VA_ARGS__)
#define AETHER_CORE_ERROR(...)    ::aether::Log::Print(::aether::LogLevel::Error, __VA_ARGS__)
#define AETHER_CORE_CRITICAL(...) ::aether::Log::Print(::aether::LogLevel::Critical, __VA_ARGS__)

#ifdef AETHER_DEBUG
// Fix: Using {0} in the format string to match the AETHER_CORE_ERROR expectation
#define AETHER_ASSERT(x, ...) { if(!(x)) { AETHER_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); AETHER_DEBUGBREAK(); } }
#else
#define AETHER_ASSERT(x, ...)
#endif
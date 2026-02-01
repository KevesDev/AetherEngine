#pragma once
#include <string>
#include <format> // C++20 standard
#include <iostream>

// --- Platform Detection & Breakpoint Macro ---
#if defined(_MSC_VER)
    // Windows (Visual Studio)
#define AETHER_DEBUGBREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
    // Linux & Mac (GCC / Clang)
#define AETHER_DEBUGBREAK() __builtin_trap()
#else
    // Fallback for unknown compilers
#include <cstdlib>
#define AETHER_DEBUGBREAK() std::abort()
#endif

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

    class Log
    {
    public:
        static void Write(LogLevel level, const std::string& message);

        // Template helper for formatted printing
        // Usage: AETHER_CORE_INFO("Loaded texture: {0} ({1}x{2})", name, width, height);
        template<typename... Args>
        static void Print(LogLevel level, std::format_string<Args...> fmt, Args&&... args)
        {
            Write(level, std::format(fmt, std::forward<Args>(args)...));
        }
    };
}

// --- Logging Macros ---
// We use '::aether' to ensure we don't have namespace collisions
#define AETHER_CORE_TRACE(...)    ::aether::Log::Print(::aether::LogLevel::Debug, __VA_ARGS__)
#define AETHER_CORE_INFO(...)     ::aether::Log::Print(::aether::LogLevel::Info, __VA_ARGS__)
#define AETHER_CORE_WARN(...)     ::aether::Log::Print(::aether::LogLevel::Warning, __VA_ARGS__)
#define AETHER_CORE_ERROR(...)    ::aether::Log::Print(::aether::LogLevel::Error, __VA_ARGS__)
#define AETHER_CORE_CRITICAL(...) ::aether::Log::Print(::aether::LogLevel::Critical, __VA_ARGS__)

// --- Assertion Macro ---
// Checks condition 'x'. If false, logs error and breaks into debugger.
#ifdef AETHER_DEBUG
#define AETHER_ASSERT(x, ...) { if(!(x)) { AETHER_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); AETHER_DEBUGBREAK(); } }
#else
    // Stripped out in Release builds for performance
#define AETHER_ASSERT(x, ...)
#endif
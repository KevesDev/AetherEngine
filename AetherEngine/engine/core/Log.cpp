#include "Log.h"
#include <chrono>
#include <iostream>
#include <filesystem>

namespace aether {

    std::vector<LogEntry> Log::s_LogHistory;
    std::ofstream Log::s_LogFile;
    std::mutex Log::s_LogMutex;

    void Log::Init() {
        std::lock_guard<std::mutex> lock(s_LogMutex);

        // Industry Standard: Logs should be in the execution directory
        s_LogFile.open("AetherLog.log", std::ios::out | std::ios::trunc);

        if (s_LogFile.is_open()) {
            s_LogFile << "--- Aether Engine Log Started ---" << std::endl;
        }

        s_LogHistory.reserve(1000);
    }

    void Log::Write(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(s_LogMutex);

        auto now = std::chrono::system_clock::now();
        std::string timeStr = std::format("{:%H:%M:%S}", now);

        LogEntry entry = { level, message, timeStr };
        s_LogHistory.push_back(entry);

        std::string levelStr;
        switch (level) {
        case LogLevel::Debug:    levelStr = "[DEBUG]"; break;
        case LogLevel::Info:     levelStr = "[INFO] "; break;
        case LogLevel::Warning:  levelStr = "[WARN] "; break;
        case LogLevel::Error:    levelStr = "[ERROR]"; break;
        case LogLevel::Critical: levelStr = "[CRIT] "; break;
        }

        std::string formatted = std::format("{} {} {}", timeStr, levelStr, message);

        // Output to terminal
        std::cout << formatted << std::endl;

        // Output to file and FORCE disk write
        if (s_LogFile.is_open()) {
            s_LogFile << formatted << std::endl;
            s_LogFile.flush(); // This is the "Anti-Blink" fix
        }
    }
}
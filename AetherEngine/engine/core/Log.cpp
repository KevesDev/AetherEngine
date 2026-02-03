#include "Log.h"
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace aether {

    std::vector<LogEntry> Log::s_LogHistory;
    std::ofstream Log::s_LogFile;
    std::mutex Log::s_LogMutex;

    void Log::Init() {
        std::lock_guard<std::mutex> lock(s_LogMutex);
        s_LogFile.open("AetherLog.log", std::ios::out | std::ios::trunc);

        if (s_LogFile.is_open()) {
            s_LogFile << "--- Aether Engine Log Started ---" << std::endl;
        }

        s_LogHistory.reserve(1000);
    }

    void Log::Write(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(s_LogMutex);

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        // Standardizing time format for cross-platform log consistency
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        std::string timeStr = ss.str();

        std::string levelStr;
        switch (level) {
        case LogLevel::Debug:    levelStr = "[TRACE]"; break;
        case LogLevel::Info:     levelStr = "[INFO] "; break;
        case LogLevel::Warning:  levelStr = "[WARN] "; break;
        case LogLevel::Error:    levelStr = "[ERROR]"; break;
        case LogLevel::Critical: levelStr = "[CRIT] "; break;
        }

        std::string formatted = std::format("{} {} {}", timeStr, levelStr, message);

        std::cout << formatted << std::endl;

        if (s_LogFile.is_open()) {
            s_LogFile << formatted << std::endl;
            s_LogFile.flush();
        }

        LogEntry entry = { level, message, timeStr };
        s_LogHistory.push_back(entry);
    }
}
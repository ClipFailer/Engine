/**
 * @file Log.cpp
 * @brief Implementation of the Logger subsystem.
 */

#include "Log.hpp"
#include <iomanip>
#include <ctime>

namespace Engine {

    void Logger::Log(LogLevel level, std::string_view msg) {
        std::lock_guard<std::mutex> lock(s_Mutex);

        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::tm timeInfo{};
#if defined(_WIN32)
        localtime_s(&timeInfo, &in_time_t);
#else
        localtime_r(&in_time_t, &timeInfo);
#endif
        std::stringstream timeStream;
        timeStream << std::put_time(&timeInfo, "%H:%M:%S");
        std::string timeStr = timeStream.str();

        std::string prefix;
        switch (level) {
            case LogLevel::Info:    prefix = "[INFO]   "; break;
            case LogLevel::Warning: prefix = "[WARN]   "; break;
            case LogLevel::Error:   prefix = "[ERROR]  "; break;
        }

        std::cout << "[" << timeStr << "] " << prefix << msg << "\n";
        s_Messages.push_back({level, std::move(timeStr), std::string(msg)});

        // Limit buffer size to prevent memory bloat
        if (s_Messages.size() > 5000) {
            s_Messages.erase(s_Messages.begin(), s_Messages.begin() + 500);
        }
    }

    std::vector<LogMessage> Logger::GetMessages() {
        std::lock_guard<std::mutex> lock(s_Mutex);
        return s_Messages;
    }

    void Logger::Clear() {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_Messages.clear();
    }

} // namespace Engine

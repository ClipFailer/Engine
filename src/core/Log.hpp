/**
 * @file Log.hpp
 * @brief Thread-safe logging subsystem with severity levels and in-memory log buffer for the editor.
 */

#pragma once

#include "Types.hpp"
#include <string>
#include <vector>
#include <mutex>
#include <sstream>
#include <iostream>
#include <chrono>

namespace Engine {

    /**
     * @brief Severity levels for log entries.
     */
    enum class LogLevel : uint8 {
        Info,
        Warning,
        Error
    };

    /**
     * @struct LogMessage
     * @brief Represents a single logged message with timestamp and severity.
     */
    struct LogMessage {
        LogLevel Level{LogLevel::Info};
        std::string Time;
        std::string Message;
    };

    /**
     * @class Logger
     * @brief Centralized logging manager.
     */
    class Logger {
    public:
        /**
         * @brief Logs a formatted message.
         * @param level Severity of the message.
         * @param msg Text message.
         */
        static void Log(LogLevel level, std::string_view msg);

        /**
         * @brief Returns copy of accumulated log entries for editor console display.
         * @return Vector of log messages.
         */
        static std::vector<LogMessage> GetMessages();

        /**
         * @brief Clears all accumulated log messages.
         */
        static void Clear();

    private:
        static inline std::mutex s_Mutex;
        static inline std::vector<LogMessage> s_Messages;
    };

} // namespace Engine

#define LOG_INFO(...)  ::Engine::Logger::Log(::Engine::LogLevel::Info,    std::format(__VA_ARGS__))
#define LOG_WARN(...)  ::Engine::Logger::Log(::Engine::LogLevel::Warning, std::format(__VA_ARGS__))
#define LOG_ERROR(...) ::Engine::Logger::Log(::Engine::LogLevel::Error,   std::format(__VA_ARGS__))

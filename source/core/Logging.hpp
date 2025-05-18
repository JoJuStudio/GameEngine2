// source/core/Logging.hpp

#pragma once
#include <stdio.h>
#include <switch.h>


class Logger {
public:
    enum class Level {
        Info,
        Warning,
        Error,
        Debug,
        None
    };

    // Initialize WITH file logging by default

    static void Initialize(Level minLevel = Level::Info, const char* logFilePath = "sdmc:/GameEngine2.log");
    static void Shutdown();
    static void Log(Level level, const char* format, ...);

    // Optional method to disable file logging
    static void DisableFileLogging();

private:
    static Level m_minLevel;
    static FILE* m_logFile;
    static Mutex m_mutex;
    static bool m_fileLoggingEnabled;

    static const char* LevelToString(Level level);
    static void Output(Level level, const char* message);
};

// Original interface wrappers
inline void initLogging() { Logger::Initialize(); }
inline void LoggingExit() { Logger::Shutdown(); }

// Original macros
#define LOG_INFO(fmt, ...)  Logger::Log(Logger::Level::Info, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  Logger::Log(Logger::Level::Warning, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::Log(Logger::Level::Error, fmt, ##__VA_ARGS__)

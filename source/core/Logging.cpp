#include "Logging.hpp"

// Initialize static members
Logger::Level Logger::m_minLevel = Logger::Level::Info;
FILE* Logger::m_logFile = nullptr;
Mutex Logger::m_mutex = 0;
bool Logger::m_fileLoggingEnabled = true;

// Initialization WITH file logging by default
void Logger::Initialize(Level minLevel, const char* logFilePath) {
    mutexLock(&m_mutex);
    socketInitializeDefault();
    nxlinkStdio();
    m_minLevel = minLevel;

    // Default file logging
    if(m_fileLoggingEnabled && logFilePath) {
        m_logFile = fopen(logFilePath, "w");
        if(m_logFile) {
            setvbuf(m_logFile, nullptr, _IOLBF, 0);
            printf("[INFO] Logging initialized (console + file: %s)\n", logFilePath);
        }
    }

    mutexUnlock(&m_mutex);
}

void Logger::DisableFileLogging() {
    mutexLock(&m_mutex);
    if(m_logFile) {
        fclose(m_logFile);
        m_logFile = nullptr;
    }
    m_fileLoggingEnabled = false;
    printf("[INFO] File logging disabled\n");
    mutexUnlock(&m_mutex);
}

void Logger::Shutdown() {
    mutexLock(&m_mutex);
    printf("[INFO] Shutting down logging\n");

    if(m_logFile) {
        fclose(m_logFile);
        m_logFile = nullptr;
    }

    socketExit();
    mutexUnlock(&m_mutex);
}


void Logger::Log(Level level, const char* format, ...) {
    if(level < m_minLevel) return;

    va_list args;
    va_start(args, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    Output(level, message);
}

const char* Logger::LevelToString(Level level) {
    switch(level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        default:            return "UNKNOWN";
    }
}

void Logger::Output(Level level, const char* message) {
    mutexLock(&m_mutex);

    // Original format: [LEVEL] message
    printf("[%s] %s\n", LevelToString(level), message);

    if(m_logFile) {
        fprintf(m_logFile, "[%s] %s\n", LevelToString(level), message);
        fflush(m_logFile);
    }

    mutexUnlock(&m_mutex);
}

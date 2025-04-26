#pragma once
#include <cstdio>
#include <switch.h>

inline void initLogging()
{
    socketInitializeDefault();
    nxlinkStdio();
    printf("[INFO] Logging initialized\n");
}

inline void LoggingExit()
{
    printf("[INFO] Shutting down logging\n");
    socketExit();
}

#define LOG_INFO(fmt, ...)                         \
    do {                                           \
        printf("[INFO] " fmt "\n", ##__VA_ARGS__); \
    } while (0)
#define LOG_WARN(fmt, ...)                         \
    do {                                           \
        printf("[WARN] " fmt "\n", ##__VA_ARGS__); \
    } while (0)
#define LOG_ERROR(fmt, ...)                         \
    do {                                            \
        printf("[ERROR] " fmt "\n", ##__VA_ARGS__); \
    } while (0)

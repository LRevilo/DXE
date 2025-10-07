#pragma once
#include "DXE.h"
#include <string>
#include <sstream>
#include <iostream>
#define NOMINMAX
#include <windows.h>

namespace DXE
{
    enum class LogLevel {
        Log,
        Info,
        Warning,
        Error
    };

    class DXE_API Logger {
    public:
        static Logger* s_Logger;
        static Logger* Get() { return s_Logger; }
        std::string name = "DXLogger";
        static void Init(Logger* logger = nullptr, const char* src = nullptr);

        const char* source = "";

        static void AllocateConsole();
        static void Log(const char* src, LogLevel level, const std::string& message) {
            Log(src, level, message, "");
        }
        template <typename T>
        static void Log(const char* src, LogLevel level, const std::string& message, const T& extraArg) {
            // Reset color after printing
            std::string resetCode = "\033[0m";
            std::string colorCode;
            std::string logLevel;
            std::stringstream logStream;

            switch (level) {
            case LogLevel::Info:
                colorCode = "\033[32m";  // Green for Info
                logLevel = "Info";
                break;
            case LogLevel::Warning: 
                colorCode = "\033[33m";  // Yellow for Warning
                logLevel = "Warn";
                break;
            case LogLevel::Error: 
                colorCode = "\033[31m";  // Red for Error
                logLevel = "Err ";
                break;
            default: 
                colorCode= "\033[0m";
                logLevel = "Log ";
                break;
            }

            auto log_source = src;
            // yuck what even is this. (not sorry)
            if (Logger::Get() && Logger::Get()->source && Logger::Get()->source[0] != '\0') { log_source = Logger::Get()->source; }
            // Format message and extraArg using stringstream
            // Handle extraArg: Use a generic way to print anything that has `<<` operator
            logStream << "[" << log_source << ":" << colorCode << logLevel << resetCode << "] " << message << "";
            logStream << extraArg;

            // Output log
            if (level == LogLevel::Error) {std::cerr << logStream.str() << std::endl;}
            else { std::cout << logStream.str() << std::endl;}
        }
    };
    
}

#ifndef LOG_SOURCE
#if defined(DXENGINE)
#define LOG_SOURCE "\033[90mDXE\033[0m"
#elif defined(DXAPP)
#define LOG_SOURCE "\033[34mAPP\033[0m"
#elif defined(DXLAYER)
#define LOG_SOURCE "\033[36mLAY\033[0m"
#else
#define LOG_SOURCE "\033[90m???\033[0m"
#endif
#endif

#ifdef _DEBUG
#define DXE_LOG(...)       ::DXE::Logger::Log(LOG_SOURCE,DXE::LogLevel::Log,__VA_ARGS__)
#define DXE_INFO(...)      ::DXE::Logger::Log(LOG_SOURCE,DXE::LogLevel::Info,__VA_ARGS__)
#define DXE_WARN(...)      ::DXE::Logger::Log(LOG_SOURCE,DXE::LogLevel::Warning,__VA_ARGS__)
#define DXE_ERROR(...)     ::DXE::Logger::Log(LOG_SOURCE,DXE::LogLevel::Error,__VA_ARGS__)


#else
#define DXE_LOG(...)     ((void)0)
#define DXE_INFO(...)      ((void)0)
#define DXE_WARN(...)      ((void)0)
#define DXE_ERROR(...)     ((void)0)
#define DXE_SET_SOURCE
#define DXE_RESET_SOURCE 
#endif
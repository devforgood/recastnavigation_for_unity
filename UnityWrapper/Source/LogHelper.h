#pragma once

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <cstdarg>
#include <ctime>
#include <string>

class LogHelper
{
private:
    static std::ofstream s_logFile;
    static bool s_logInitialized;
    static std::string s_logDirectory;

public:
    // Initialize logging system
    static void Initialize(const std::string& logDir = "logs");
    
    // Cleanup logging system
    static void Cleanup();
    
    // Custom printf function that writes to both console and log file
    static void LogPrintf(const char* format, ...);
    
    // Check if logging is initialized
    static bool IsInitialized() { return s_logInitialized; }
    
    // Get current log file path
    static std::string GetLogFilePath();
    
    // Write a simple message to log
    static void LogMessage(const std::string& message);
    
    // Write error message to log
    static void LogError(const std::string& error);
    
    // Write warning message to log
    static void LogWarning(const std::string& warning);
    
    // Write info message to log
    static void LogInfo(const std::string& info);
    
    // Write debug message to log
    static void LogDebug(const std::string& debug);
    
    // Flush log buffer
    static void Flush();
}; 
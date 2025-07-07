#include "LogHelper.h"

// Static member initialization
std::ofstream LogHelper::s_logFile;
bool LogHelper::s_logInitialized = false;
std::string LogHelper::s_logDirectory = "logs";

void LogHelper::Initialize(const std::string& logDir)
{
    if (!s_logInitialized)
    {
        s_logDirectory = logDir;
        
        // Create logs directory if it doesn't exist
        #ifdef _WIN32
        std::string mkdirCmd = "mkdir " + logDir + " 2>nul";
        system(mkdirCmd.c_str());
        #else
        std::string mkdirCmd = "mkdir -p " + logDir + " 2>/dev/null";
        system(mkdirCmd.c_str());
        #endif
        
        // Open log file with timestamp
        time_t now = time(0);
        struct tm* ltm = localtime(&now);
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/recastnavigation_%04d%02d%02d_%02d%02d%02d.log",
                logDir.c_str(),
                1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        
        s_logFile.open(filename, std::ios::out | std::ios::app);
        if (s_logFile.is_open())
        {
            s_logFile << "=== RecastNavigation Log Started ===" << std::endl;
            s_logFile << "Timestamp: " << ctime(&now);
            s_logFile.flush();
            s_logInitialized = true;
            printf("Logging initialized: %s\n", filename);
        }
        else
        {
            printf("Failed to open log file: %s\n", filename);
        }
    }
}

void LogHelper::Cleanup()
{
    if (s_logFile.is_open())
    {
        s_logFile << "=== RecastNavigation Log Ended ===" << std::endl;
        s_logFile.close();
        s_logInitialized = false;
    }
}

void LogHelper::LogPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    // Get the formatted string length
    va_list args_copy;
    va_copy(args_copy, args);
    int length = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);
    
    if (length > 0)
    {
        // Allocate buffer for the formatted string
        char* buffer = new char[length + 1];
        vsnprintf(buffer, length + 1, format, args);
        
        // Print to console
        printf("%s", buffer);
        
        // Write to log file if initialized
        if (s_logInitialized && s_logFile.is_open())
        {
            s_logFile << buffer;
            s_logFile.flush(); // Ensure immediate write
        }
        
        delete[] buffer;
    }
    
    va_end(args);
}

std::string LogHelper::GetLogFilePath()
{
    if (s_logInitialized && s_logFile.is_open())
    {
        // Note: This is a simplified approach. In a real implementation,
        // you might want to store the actual file path.
        time_t now = time(0);
        struct tm* ltm = localtime(&now);
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/recastnavigation_%04d%02d%02d_%02d%02d%02d.log",
                s_logDirectory.c_str(),
                1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
                ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
        return std::string(filename);
    }
    return "";
}

void LogHelper::LogMessage(const std::string& message)
{
    LogPrintf("[MESSAGE] %s\n", message.c_str());
}

void LogHelper::LogError(const std::string& error)
{
    LogPrintf("[ERROR] %s\n", error.c_str());
}

void LogHelper::LogWarning(const std::string& warning)
{
    LogPrintf("[WARNING] %s\n", warning.c_str());
}

void LogHelper::LogInfo(const std::string& info)
{
    LogPrintf("[INFO] %s\n", info.c_str());
}

void LogHelper::LogDebug(const std::string& debug)
{
    LogPrintf("[DEBUG] %s\n", debug.c_str());
}

void LogHelper::Flush()
{
    if (s_logInitialized && s_logFile.is_open())
    {
        s_logFile.flush();
    }
} 
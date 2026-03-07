#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>

// Log severity levels
enum LogLevel {
    LOG_INFO  = 0,
    LOG_WARN  = 1,
    LOG_ERROR = 2
};

// Logger - Records every FS operation to fs_log.txt with timestamp, level,
// operation name, and a detail message.  Also supports printing the log to
// the console via the 'log' shell command.
class Logger {
private:
    std::string logFilePath;
    std::ofstream logFile;
    bool isOpen;

    // Format the current time as a human-readable string
    std::string currentTimestamp() const;

    // Convert LogLevel to string tag
    std::string levelTag(LogLevel level) const;

public:
    // Constructor - opens (or creates) the log file
    Logger(const std::string& filePath = "fs_log.txt");

    // Destructor - closes the log file
    ~Logger();

    // Log an entry: log(LOG_INFO, "MKDIR", "name='docs' inode=3 SUCCESS")
    void log(LogLevel level, const std::string& operation, const std::string& details);

    // Convenience wrappers
    void info (const std::string& operation, const std::string& details);
    void warn (const std::string& operation, const std::string& details);
    void error(const std::string& operation, const std::string& details);

    // Print all log contents to stdout (used by 'log' shell command)
    void printLog() const;

    // Clear the log file
    void clearLog();

    // Check if logger is working
    bool isWorking() const;
};

#endif // LOGGER_H

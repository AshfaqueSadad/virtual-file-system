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

// Logger — records every FS operation to fs_log.txt with timestamp, level,
// operation name, and a detail message.
class Logger {
private:
    std::string   logFilePath;
    std::ofstream logFile;
    bool          isOpen;
    unsigned int  entryCount;   // Total lines written since construction

    // Return current wall-clock time as "YYYY-MM-DD HH:MM:SS"
    std::string currentTimestamp() const;

    // Map a LogLevel to its short string tag
    std::string levelTag(LogLevel level) const;

public:
    Logger(const std::string& filePath = "fs_log.txt");
    ~Logger();

    // Core log call
    void log(LogLevel level, const std::string& operation, const std::string& details);

    // Convenience wrappers
    void info (const std::string& operation, const std::string& details);
    void warn (const std::string& operation, const std::string& details);
    void error(const std::string& operation, const std::string& details);

    // Print all log contents to stdout (used by the 'log' shell command)
    void printLog() const;

    // Erase the log file
    void clearLog();

    // Return true if the log file opened successfully
    bool isWorking() const;

    // Return the number of log entries written so far
    unsigned int getEntryCount() const;
};

#endif // LOGGER_H

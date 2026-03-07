#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

// Constructor - opens the log file in append mode so previous sessions are kept
Logger::Logger(const string& filePath)
    : logFilePath(filePath), isOpen(false) {

    logFile.open(logFilePath, ios::out | ios::app);

    if (logFile.is_open()) {
        isOpen = true;
        // Write a session separator so logs from different runs are distinct
        logFile << "\n";
        logFile << "============================================================\n";
        logFile << "  SESSION STARTED  at " << currentTimestamp() << "\n";
        logFile << "============================================================\n";
        logFile.flush();
        // cout << "[Logger] Log file opened: " << logFilePath << endl;
    } else {
        cerr << "[Logger] WARNING: Could not open log file '" << logFilePath
             << "'. Logging disabled." << endl;
    }
}

// Destructor - close the file cleanly
Logger::~Logger() {
    if (isOpen) {
        logFile << "============================================================\n";
        logFile << "  SESSION ENDED    at " << currentTimestamp() << "\n";
        logFile << "============================================================\n";
        logFile.close();
    }
    // cout << "[Logger] Destroyed" << endl;
}

// Returns a formatted timestamp string: "YYYY-MM-DD HH:MM:SS"
string Logger::currentTimestamp() const {
    time_t now = time(nullptr);
    struct tm* tmInfo = localtime(&now);

    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmInfo);
    return string(buf);
}

// Convert severity level to a fixed-width tag
string Logger::levelTag(LogLevel level) const {
    switch (level) {
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "INFO ";
    }
}

// Core logging function
void Logger::log(LogLevel level, const string& operation, const string& details) {
    if (!isOpen) return;

    // Format: [2026-02-21 11:03:01] [INFO ] MKDIR  name='docs' inode=3 SUCCESS
    string line = "[" + currentTimestamp() + "] "
                + "[" + levelTag(level) + "] "
                + operation + "  "
                + details;

    logFile << line << "\n";
    logFile.flush();
}

// Convenience wrappers
void Logger::info(const string& operation, const string& details) {
    log(LOG_INFO, operation, details);
}

void Logger::warn(const string& operation, const string& details) {
    log(LOG_WARN, operation, details);
}

void Logger::error(const string& operation, const string& details) {
    log(LOG_ERROR, operation, details);
}

// Print all log entries to stdout
void Logger::printLog() const {
    ifstream reader(logFilePath);

    if (!reader.is_open()) {
        cout << "ERROR: Cannot open log file '" << logFilePath << "'" << endl;
        return;
    }

    cout << "\n=== OPERATION LOG (" << logFilePath << ") ===" << endl;

    string line;
    bool hasContent = false;
    while (getline(reader, line)) {
        cout << line << "\n";
        hasContent = true;
    }

    if (!hasContent) {
        cout << "(log is empty)" << endl;
    }

    cout << "=== END OF LOG ===" << endl;
    reader.close();
}

// Clear the log file
void Logger::clearLog() {
    logFile.close();
    logFile.open(logFilePath, ios::out | ios::trunc);  // truncate = erase contents
    isOpen = logFile.is_open();
    if (isOpen) {
        logFile << "  LOG CLEARED at " << currentTimestamp() << "\n";
        logFile.flush();
        cout << "[Logger] Log cleared." << endl;
    }
}

bool Logger::isWorking() const {
    return isOpen;
}

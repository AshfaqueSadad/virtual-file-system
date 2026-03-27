#include "Logger.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

Logger::Logger(const string& filePath)
    : logFilePath(filePath), isOpen(false), entryCount(0) {

    logFile.open(logFilePath, ios::out | ios::app);

    if (logFile.is_open()) {
        isOpen = true;
        logFile << "\n";
        logFile << "============================================================\n";
        logFile << "  SESSION STARTED  " << currentTimestamp() << "\n";
        logFile << "============================================================\n";
        logFile.flush();
    } else {
        cerr << "[Logger] WARNING: Cannot open '" << logFilePath << "'. Logging disabled." << endl;
    }
}

Logger::~Logger() {
    if (isOpen) {
        logFile << "============================================================\n";
        logFile << "  SESSION ENDED    " << currentTimestamp() << "\n";
        logFile << "============================================================\n";
        logFile.close();
    }
}

// Returns "YYYY-MM-DD HH:MM:SS"
string Logger::currentTimestamp() const {
    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", t);
    return string(buf);
}

string Logger::levelTag(LogLevel level) const {
    switch (level) {
        case LOG_INFO:  return "INFO ";
        case LOG_WARN:  return "WARN ";
        case LOG_ERROR: return "ERROR";
        default:        return "INFO ";
    }
}

void Logger::log(LogLevel level, const string& operation, const string& details) {
    if (!isOpen) return;
    string line = "[" + currentTimestamp() + "] "
                + "[" + levelTag(level) + "] "
                + operation + "  " + details;
    logFile << line << "\n";
    logFile.flush();
    entryCount++;
}

void Logger::info (const string& op, const string& d) { log(LOG_INFO,  op, d); }
void Logger::warn (const string& op, const string& d) { log(LOG_WARN,  op, d); }
void Logger::error(const string& op, const string& d) { log(LOG_ERROR, op, d); }

void Logger::printLog() const {
    ifstream reader(logFilePath);
    if (!reader.is_open()) {
        cout << "ERROR: Cannot open log file '" << logFilePath << "'" << endl;
        return;
    }
    cout << "\n=== OPERATION LOG (" << logFilePath << ") ===" << endl;
    string line;
    bool hasContent = false;
    while (getline(reader, line)) { cout << line << "\n"; hasContent = true; }
    if (!hasContent) cout << "(log is empty)" << endl;
    cout << "=== END OF LOG ===" << endl;
    reader.close();
}

void Logger::clearLog() {
    logFile.close();
    logFile.open(logFilePath, ios::out | ios::trunc);
    isOpen      = logFile.is_open();
    entryCount  = 0;
    if (isOpen) {
        logFile << "  LOG CLEARED " << currentTimestamp() << "\n";
        logFile.flush();
    }
}

bool Logger::isWorking() const { return isOpen; }

unsigned int Logger::getEntryCount() const { return entryCount; }

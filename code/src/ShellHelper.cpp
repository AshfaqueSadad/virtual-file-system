#include "ShellHelper.h"
#include <iostream>
#include <sstream>

using namespace std;

void ShellHelper::printBanner() {
    cout << "===========================================" << endl;
    cout << "  EXT-2 FILE SYSTEM SIMULATOR" << endl;
    cout << "===========================================" << endl;
}

void ShellHelper::printHelp() {
    cout << "\n=== EXT-2 FILE SYSTEM COMMANDS ===" << endl;
    cout << "format                    - Format the file system (WARNING: erases all data)" << endl;
    cout << "mkdir <path>              - Create a new directory" << endl;
    cout << "touch <file>              - Create a new empty file" << endl;
    cout << "ls [path]                 - List directory contents" << endl;
    cout << "cd <path>                 - Change current directory (supports cd ..)" << endl;
    cout << "pwd                       - Print working directory" << endl;
    cout << "write <file> <text>       - Write text to a file" << endl;
    cout << "read <file>               - Read and display file contents" << endl;
    cout << "rm <path>                 - Remove a file or empty directory" << endl;
    cout << "cp <source> <dest>        - Copy a file to a new name" << endl;
    cout << "mv <source> <dest>        - Rename a file or directory" << endl;
    cout << "find <name> [-f|-d]       - Search entire FS for a file/dir by name" << endl;
    cout << "                           (-f = files only, -d = directories only)" << endl;
    cout << "info                      - Show file system statistics" << endl;
    cout << "log                       - Show operation log" << endl;
    cout << "log clear                 - Clear the operation log" << endl;
    cout << "help                      - Show this help menu" << endl;
    cout << "exit                      - Exit the file system simulator" << endl;
    cout << "===================================\n" << endl;
}

void ShellHelper::printDirEntry(const DirectoryEntry& entry) {
    string typeTag = (entry.fileType == TYPE_DIRECTORY) ? "[DIR] " : "[FILE]";
    cout << typeTag << " " << entry.name << endl;
}

string ShellHelper::formatSize(unsigned int bytes) {
    ostringstream oss;
    if (bytes >= 1024 * 1024) {
        oss << (bytes / (1024 * 1024)) << " MB";
    } else if (bytes >= 1024) {
        oss << (bytes / 1024) << " KB";
    } else {
        oss << bytes << " B";
    }
    return oss.str();
}

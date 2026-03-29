#ifndef SHELLHELPER_H
#define SHELLHELPER_H

#include "Ext2Structs.h"
#include <string>

// ShellHelper — pure static display utilities for the interactive shell.
// Keeping all console-formatting logic here lets main.cpp stay focused on
// command dispatch and filesystem coordination.
class ShellHelper {
public:
    // Print the startup banner
    static void printBanner();

    // Print the help menu listing all supported commands
    static void printHelp();

    // Format and print a single directory entry line (used by ls)
    static void printDirEntry(const DirectoryEntry& entry);

    // Format a file-size number into a human-readable string (e.g. "2 KB")
    static std::string formatSize(unsigned int bytes);
};

#endif // SHELLHELPER_H

#ifndef DIRECTORYHANDLER_H
#define DIRECTORYHANDLER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "InodeManager.h"
#include "BlockManager.h"
#include "FileManager.h"
#include "PathParser.h"
#include <string>
#include <vector>

class DirectoryHandler {
private:
    VirtualDisk* disk;
    InodeManager* inodeMgr;
    BlockManager* blockMgr;
    FileManager* fileMgr;

    // Internal recursive search helper
    void searchRecursive(unsigned int dirInode,
                         const std::string& target,
                         const std::string& currentPath,
                         bool matchFiles,
                         bool matchDirs,
                         std::vector<std::string>& results);

public:
    // Constructor
    DirectoryHandler(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                    BlockManager* blockMgr, FileManager* fileMgr);

    // Destructor
    ~DirectoryHandler();

    // Create a new directory (returns inode number or -1 on failure)
    int createDirectory();

    // Delete a directory (must be empty)
    bool deleteDirectory(unsigned int inodeNumber);

    // Add entry to directory
    bool addEntry(unsigned int dirInodeNumber, const std::string& name,
                  unsigned int entryInodeNumber, InodeType type);

    // Remove entry from directory
    bool removeEntry(unsigned int dirInodeNumber, const std::string& name);

    // Find entry in directory (returns inode number or -1 if not found)
    int findEntry(unsigned int dirInodeNumber, const std::string& name);

    // List directory contents
    std::vector<DirectoryEntry> listDirectory(unsigned int dirInodeNumber);

    // Check if directory is empty
    bool isEmpty(unsigned int dirInodeNumber);

    // Get directory entry count
    unsigned int getEntryCount(unsigned int dirInodeNumber);

    // NEW: Search entire FS tree for entries matching 'name'.
    // matchFiles / matchDirs control what types are reported.
    // Returns a list of full paths to all matches.
    std::vector<std::string> search(const std::string& name,
                                    bool matchFiles = true,
                                    bool matchDirs  = true);
};

#endif // DIRECTORYHANDLER_H

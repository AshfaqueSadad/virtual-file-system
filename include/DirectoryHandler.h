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
};

#endif // DIRECTORYHANDLER_H

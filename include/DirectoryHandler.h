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
    VirtualDisk*  disk;
    InodeManager* inodeMgr;
    BlockManager* blockMgr;
    FileManager*  fileMgr;

    // Read all raw entry bytes for a directory into a heap buffer.
    // Caller must delete[] the returned pointer (or it is nullptr on failure).
    char* loadEntries(unsigned int dirInode, unsigned int& outSize);

    // Write a raw entry buffer back and update the directory inode on disk.
    bool flushEntries(unsigned int dirInode, const char* buf,
                      unsigned int newSize, Inode& dirInodeData);

    // Recursive DFS used by search()
    void searchRecursive(unsigned int dirInode,
                         const std::string& target,
                         const std::string& currentPath,
                         bool matchFiles,
                         bool matchDirs,
                         std::vector<std::string>& results);

public:
    DirectoryHandler(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                     BlockManager* blockMgr, FileManager* fileMgr);
    ~DirectoryHandler();

    // Allocate a new directory inode — returns inode number or -1
    int  createDirectory();

    // Free a directory (must be empty first)
    bool deleteDirectory(unsigned int inodeNumber);

    // Add a named entry pointing to entryInodeNumber
    bool addEntry(unsigned int dirInodeNumber, const std::string& name,
                  unsigned int entryInodeNumber, InodeType type);

    // Remove a named entry from a directory
    bool removeEntry(unsigned int dirInodeNumber, const std::string& name);

    // Return inode number for 'name' in directory, or -1 if not found
    int  findEntry(unsigned int dirInodeNumber, const std::string& name);

    // Return all entries in a directory
    std::vector<DirectoryEntry> listDirectory(unsigned int dirInodeNumber);

    // Return true if the directory contains no entries
    bool isEmpty(unsigned int dirInodeNumber);

    // Return the number of entries in a directory
    unsigned int getEntryCount(unsigned int dirInodeNumber);

    // Walk the entire FS tree searching for entries named 'name'
    std::vector<std::string> search(const std::string& name,
                                    bool matchFiles = true,
                                    bool matchDirs  = true);
};

#endif // DIRECTORYHANDLER_H

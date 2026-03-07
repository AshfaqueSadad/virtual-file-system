#include "DirectoryHandler.h"
#include <cstring>

using namespace std;

// Constructor
DirectoryHandler::DirectoryHandler(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                                   BlockManager* blockMgr, FileManager* fileMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr), fileMgr(fileMgr) {
    // cout << "[DirectoryHandler] Initialized" << endl;
}

// Destructor
DirectoryHandler::~DirectoryHandler() {
    // cout << "[DirectoryHandler] Destroyed" << endl;
}

// Create a new directory
int DirectoryHandler::createDirectory() {
    // cout << "[DirectoryHandler] Creating new directory" << endl;

    int inodeNumber = inodeMgr->allocateInode(TYPE_DIRECTORY);

    if (inodeNumber == -1) {
        cerr << "[DirectoryHandler] ERROR: Failed to allocate inode for directory" << endl;
        return -1;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to read newly created inode" << endl;
        inodeMgr->deallocateInode(inodeNumber);
        return -1;
    }

    inode.size = 0;
    inode.blockCount = 0;

    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to initialize inode" << endl;
        inodeMgr->deallocateInode(inodeNumber);
        return -1;
    }

    // cout << "[DirectoryHandler] Directory created with inode #" << inodeNumber << endl;
    return inodeNumber;
}

// Delete a directory (must be empty)
bool DirectoryHandler::deleteDirectory(unsigned int inodeNumber) {
    // cout << "[DirectoryHandler] Deleting directory with inode #" << inodeNumber << endl;

    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }

    if (!isEmpty(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Directory not empty" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to read inode" << endl;
        return false;
    }

    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
        }
    }

    if (!inodeMgr->deallocateInode(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Failed to deallocate inode" << endl;
        return false;
    }

    // cout << "[DirectoryHandler] Directory deleted successfully" << endl;
    return true;
}

// Add an entry to a directory
bool DirectoryHandler::addEntry(unsigned int dirInodeNumber, const string& name,
                               unsigned int entryInodeNumber, InodeType type) {
    // cout << "[DirectoryHandler] Adding entry '" << name << "' to directory (inode #"
    //      << dirInodeNumber << ")" << endl;

    if (name.length() > MAX_FILENAME_LENGTH) {
        cerr << "[DirectoryHandler] ERROR: Filename too long" << endl;
        return false;
    }

    if (findEntry(dirInodeNumber, name) != -1) {
        cerr << "[DirectoryHandler] ERROR: Entry '" << name << "' already exists" << endl;
        return false;
    }

    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to read directory inode" << endl;
        return false;
    }

    // Build the new DirectoryEntry
    DirectoryEntry newEntry;
    newEntry.inodeNumber = entryInodeNumber;
    newEntry.fileType = type;
    newEntry.nameLength = name.length();
    strncpy(newEntry.name, name.c_str(), MAX_FILENAME_LENGTH);
    newEntry.name[name.length()] = '\0';
    newEntry.entryLength = sizeof(DirectoryEntry);

    unsigned int entryCount = dirInode.size / sizeof(DirectoryEntry);
    unsigned int newSize    = (entryCount + 1) * sizeof(DirectoryEntry);

    char* buffer = new char[newSize];
    memset(buffer, 0, newSize);

    // Read existing entries into buffer
    if (dirInode.size > 0) {
        unsigned int blocksToRead = (dirInode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        unsigned int bytesRead = 0;

        for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
            if (dirInode.directBlocks[i] != NULL_BLOCK) {
                unsigned int bytesToRead = (dirInode.size - bytesRead < BLOCK_SIZE) ?
                                          (dirInode.size - bytesRead) : BLOCK_SIZE;
                blockMgr->readBlock(dirInode.directBlocks[i], buffer + bytesRead, bytesToRead);
                bytesRead += bytesToRead;
            }
        }
    }

    // Append new entry
    memcpy(buffer + dirInode.size, &newEntry, sizeof(DirectoryEntry));

    unsigned int blocksNeeded = (newSize + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[DirectoryHandler] ERROR: Directory too large" << endl;
        delete[] buffer;
        return false;
    }

    // Allocate new blocks if needed
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (dirInode.directBlocks[i] == NULL_BLOCK) {
            int blockNum = blockMgr->allocateBlock();
            if (blockNum == -1) {
                cerr << "[DirectoryHandler] ERROR: Failed to allocate block" << endl;
                delete[] buffer;
                return false;
            }
            dirInode.directBlocks[i] = blockNum;
            dirInode.blockCount++;
        }
    }

    // Write data to blocks
    unsigned int bytesWritten = 0;
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        unsigned int bytesToWrite = (newSize - bytesWritten < BLOCK_SIZE) ?
                                    (newSize - bytesWritten) : BLOCK_SIZE;
        blockMgr->writeBlock(dirInode.directBlocks[i], buffer + bytesWritten, bytesToWrite);
        bytesWritten += bytesToWrite;
    }

    dirInode.size = newSize;
    dirInode.modifiedTime = time(nullptr);

    if (!inodeMgr->writeInode(dirInodeNumber, dirInode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to update directory inode" << endl;
        delete[] buffer;
        return false;
    }

    delete[] buffer;
    // cout << "[DirectoryHandler] Entry added successfully" << endl;
    return true;
}

// Remove an entry from a directory
bool DirectoryHandler::removeEntry(unsigned int dirInodeNumber, const string& name) {
    // cout << "[DirectoryHandler] Removing entry '" << name << "' from directory (inode #"
    //      << dirInodeNumber << ")" << endl;

    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);

    bool found = false;
    vector<DirectoryEntry> newEntries;

    for (const DirectoryEntry& entry : entries) {
        if (string(entry.name) == name) {
            found = true;
        } else {
            newEntries.push_back(entry);
        }
    }

    if (!found) {
        cerr << "[DirectoryHandler] ERROR: Entry '" << name << "' not found" << endl;
        return false;
    }

    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        return false;
    }

    unsigned int newSize = newEntries.size() * sizeof(DirectoryEntry);

    char* buffer = new char[newSize > 0 ? newSize : 1];
    memset(buffer, 0, newSize > 0 ? newSize : 1);

    for (size_t i = 0; i < newEntries.size(); i++) {
        memcpy(buffer + (i * sizeof(DirectoryEntry)), &newEntries[i], sizeof(DirectoryEntry));
    }

    if (newSize > 0) {
        unsigned int blocksNeeded = (newSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
        unsigned int bytesWritten = 0;

        for (unsigned int i = 0; i < blocksNeeded && i < DIRECT_BLOCKS; i++) {
            unsigned int bytesToWrite = (newSize - bytesWritten < BLOCK_SIZE) ?
                                        (newSize - bytesWritten) : BLOCK_SIZE;
            blockMgr->writeBlock(dirInode.directBlocks[i], buffer + bytesWritten, bytesToWrite);
            bytesWritten += bytesToWrite;
        }
    }

    dirInode.size = newSize;
    dirInode.modifiedTime = time(nullptr);
    inodeMgr->writeInode(dirInodeNumber, dirInode);

    delete[] buffer;
    // cout << "[DirectoryHandler] Entry removed successfully" << endl;
    return true;
}

// Find an entry by name — returns inode number or -1
int DirectoryHandler::findEntry(unsigned int dirInodeNumber, const string& name) {
    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);

    for (const DirectoryEntry& entry : entries) {
        if (string(entry.name) == name) {
            return entry.inodeNumber;
        }
    }

    return -1;
}

// List all entries in a directory
vector<DirectoryEntry> DirectoryHandler::listDirectory(unsigned int dirInodeNumber) {
    vector<DirectoryEntry> entries;

    if (!inodeMgr->inodeExists(dirInodeNumber)) {
        return entries;
    }

    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        return entries;
    }

    if (dirInode.size == 0) {
        return entries;
    }

    char* buffer = new char[dirInode.size];
    memset(buffer, 0, dirInode.size);

    unsigned int blocksToRead = (dirInode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    unsigned int bytesRead = 0;

    for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
        if (dirInode.directBlocks[i] != NULL_BLOCK) {
            unsigned int bytesToRead = (dirInode.size - bytesRead < BLOCK_SIZE) ?
                                      (dirInode.size - bytesRead) : BLOCK_SIZE;
            blockMgr->readBlock(dirInode.directBlocks[i], buffer + bytesRead, bytesToRead);
            bytesRead += bytesToRead;
        }
    }

    unsigned int entryCount = dirInode.size / sizeof(DirectoryEntry);

    for (unsigned int i = 0; i < entryCount; i++) {
        DirectoryEntry entry;
        memcpy(&entry, buffer + (i * sizeof(DirectoryEntry)), sizeof(DirectoryEntry));
        entries.push_back(entry);
    }

    delete[] buffer;
    return entries;
}

// Check if directory is empty
bool DirectoryHandler::isEmpty(unsigned int dirInodeNumber) {
    return getEntryCount(dirInodeNumber) == 0;
}

// Get entry count
unsigned int DirectoryHandler::getEntryCount(unsigned int dirInodeNumber) {
    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        return 0;
    }
    return dirInode.size / sizeof(DirectoryEntry);
}

// ── NEW: SEARCH ───────────────────────────────────────────────────────────────

// Recursive DFS helper
// dirInode     - inode of the directory being scanned
// target       - name to match (case-sensitive)
// currentPath  - path string of 'dirInode' (e.g. "/docs")
// matchFiles   - include regular files in results
// matchDirs    - include directories in results
// results      - growing list of full paths that match
void DirectoryHandler::searchRecursive(unsigned int dirInode,
                                       const string& target,
                                       const string& currentPath,
                                       bool matchFiles,
                                       bool matchDirs,
                                       vector<string>& results) {
    vector<DirectoryEntry> entries = listDirectory(dirInode);

    for (const DirectoryEntry& entry : entries) {
        string entryName = string(entry.name);
        string fullPath  = (currentPath == "/") ? ("/" + entryName)
                                                : (currentPath + "/" + entryName);

        // Check if this entry's name matches
        if (entryName == target) {
            bool isDir  = (entry.fileType == TYPE_DIRECTORY);
            bool isFile = (entry.fileType == TYPE_FILE);

            if ((isFile && matchFiles) || (isDir && matchDirs)) {
                results.push_back(fullPath);
            }
        }

        // Recurse into sub-directories
        if (entry.fileType == TYPE_DIRECTORY) {
            searchRecursive(entry.inodeNumber, target, fullPath,
                            matchFiles, matchDirs, results);
        }
    }
}

// Public search entry point — searches from root (inode 0)
vector<string> DirectoryHandler::search(const string& name,
                                        bool matchFiles,
                                        bool matchDirs) {
    vector<string> results;

    if (name.empty()) {
        return results;
    }

    // cout << "[DirectoryHandler] Searching for '" << name << "' ..." << endl;

    // Always start from root (inode 0)
    searchRecursive(0, name, "/", matchFiles, matchDirs, results);

    // cout << "[DirectoryHandler] Search complete. Found " << results.size()
    //      << " match(es)." << endl;

    return results;
}

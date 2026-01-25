#include "DirectoryHandler.h"
#include <cstring>

using namespace std;

// Constructor
DirectoryHandler::DirectoryHandler(VirtualDisk* virtualDisk, InodeManager* inodeMgr, 
                                   BlockManager* blockMgr, FileManager* fileMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr), fileMgr(fileMgr) {
    cout << "[DirectoryHandler] Initialized" << endl;
}

// Destructor
DirectoryHandler::~DirectoryHandler() {
    cout << "[DirectoryHandler] Destroyed" << endl;
}

// Create a new directory
int DirectoryHandler::createDirectory() {
    cout << "[DirectoryHandler] Creating new directory" << endl;
    
    // Allocate inode for directory
    int inodeNumber = inodeMgr->allocateInode(TYPE_DIRECTORY);
    
    if (inodeNumber == -1) {
        cerr << "[DirectoryHandler] ERROR: Failed to allocate inode for directory" << endl;
        return -1;
    }
    
    cout << "[DirectoryHandler] Directory created with inode #" << inodeNumber << endl;
    return inodeNumber;
}

// Delete a directory
bool DirectoryHandler::deleteDirectory(unsigned int inodeNumber) {
    cout << "[DirectoryHandler] Deleting directory with inode #" << inodeNumber << endl;
    
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Check if directory is empty
    if (!isEmpty(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Directory not empty" << endl;
        return false;
    }
    
    // Read inode to get block pointers
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to read inode" << endl;
        return false;
    }
    
    // Deallocate all blocks
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != 0) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
        }
    }
    
    // Deallocate inode
    if (!inodeMgr->deallocateInode(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Failed to deallocate inode" << endl;
        return false;
    }
    
    cout << "[DirectoryHandler] Directory deleted successfully" << endl;
    return true;
}

// Add entry to directory
bool DirectoryHandler::addEntry(unsigned int dirInodeNumber, const string& name, 
                               unsigned int entryInodeNumber, InodeType type) {
    cout << "[DirectoryHandler] Adding entry '" << name << "' to directory (inode #" 
         << dirInodeNumber << ")" << endl;
    
    if (name.length() > MAX_FILENAME_LENGTH) {
        cerr << "[DirectoryHandler] ERROR: Filename too long" << endl;
        return false;
    }
    
    // Check if entry already exists
    if (findEntry(dirInodeNumber, name) != -1) {
        cerr << "[DirectoryHandler] ERROR: Entry '" << name << "' already exists" << endl;
        return false;
    }
    
    // Read current directory data
    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to read directory inode" << endl;
        return false;
    }
    
    // Create new directory entry
    DirectoryEntry newEntry;
    newEntry.inodeNumber = entryInodeNumber;
    newEntry.fileType = type;
    newEntry.nameLength = name.length();
    strncpy(newEntry.name, name.c_str(), MAX_FILENAME_LENGTH);
    newEntry.name[name.length()] = '\0';
    newEntry.entryLength = sizeof(DirectoryEntry);
    
    // Read existing entries
    unsigned int entryCount = dirInode.size / sizeof(DirectoryEntry);
    unsigned int newSize = (entryCount + 1) * sizeof(DirectoryEntry);
    
    // Allocate buffer for all entries
    char* buffer = new char[newSize];
    memset(buffer, 0, newSize);
    
    // Read existing entries
    if (dirInode.size > 0) {
        unsigned int blocksToRead = (dirInode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        unsigned int bytesRead = 0;
        
        for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
            if (dirInode.directBlocks[i] != 0) {
                unsigned int bytesToRead = (dirInode.size - bytesRead < BLOCK_SIZE) ?
                                          (dirInode.size - bytesRead) : BLOCK_SIZE;
                
                blockMgr->readBlock(dirInode.directBlocks[i], buffer + bytesRead, bytesToRead);
                bytesRead += bytesToRead;
            }
        }
    }
    
    // Append new entry
    memcpy(buffer + dirInode.size, &newEntry, sizeof(DirectoryEntry));
    
    // Write back to disk
    unsigned int blocksNeeded = (newSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[DirectoryHandler] ERROR: Directory too large" << endl;
        delete[] buffer;
        return false;
    }
    
    // Allocate blocks if needed
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (dirInode.directBlocks[i] == 0) {
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
    
    // Update directory inode
    dirInode.size = newSize;
    dirInode.modifiedTime = time(nullptr);
    
    if (!inodeMgr->writeInode(dirInodeNumber, dirInode)) {
        cerr << "[DirectoryHandler] ERROR: Failed to update directory inode" << endl;
        delete[] buffer;
        return false;
    }
    
    delete[] buffer;
    cout << "[DirectoryHandler] Entry added successfully" << endl;
    return true;
}

// Remove entry from directory
bool DirectoryHandler::removeEntry(unsigned int dirInodeNumber, const string& name) {
    cout << "[DirectoryHandler] Removing entry '" << name << "' from directory (inode #" 
         << dirInodeNumber << ")" << endl;
    
    // Get all entries
    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);
    
    // Find and remove the entry
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
    
    // Write new entries back
    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) {
        return false;
    }
    
    unsigned int newSize = newEntries.size() * sizeof(DirectoryEntry);
    
    // Create buffer with new entries
    char* buffer = new char[newSize > 0 ? newSize : 1];
    
    for (size_t i = 0; i < newEntries.size(); i++) {
        memcpy(buffer + (i * sizeof(DirectoryEntry)), &newEntries[i], sizeof(DirectoryEntry));
    }
    
    // Write to blocks
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
    
    // Update inode
    dirInode.size = newSize;
    dirInode.modifiedTime = time(nullptr);
    inodeMgr->writeInode(dirInodeNumber, dirInode);
    
    delete[] buffer;
    cout << "[DirectoryHandler] Entry removed successfully" << endl;
    return true;
}

// Find entry in directory
int DirectoryHandler::findEntry(unsigned int dirInodeNumber, const string& name) {
    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);
    
    for (const DirectoryEntry& entry : entries) {
        if (string(entry.name) == name) {
            return entry.inodeNumber;
        }
    }
    
    return -1;
}

// List directory contents
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
    
    // Read directory data
    char* buffer = new char[dirInode.size];
    
    unsigned int blocksToRead = (dirInode.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    unsigned int bytesRead = 0;
    
    for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
        if (dirInode.directBlocks[i] != 0) {
            unsigned int bytesToRead = (dirInode.size - bytesRead < BLOCK_SIZE) ?
                                      (dirInode.size - bytesRead) : BLOCK_SIZE;
            
            blockMgr->readBlock(dirInode.directBlocks[i], buffer + bytesRead, bytesToRead);
            bytesRead += bytesToRead;
        }
    }
    
    // Parse entries
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

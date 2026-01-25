#include "FileManager.h"
#include <cstring>

using namespace std;

// Constructor
FileManager::FileManager(VirtualDisk* virtualDisk, InodeManager* inodeMgr, BlockManager* blockMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr) {
    cout << "[FileManager] Initialized" << endl;
}

// Destructor
FileManager::~FileManager() {
    cout << "[FileManager] Destroyed" << endl;
}

// Create a new file
int FileManager::createFile() {
    cout << "[FileManager] Creating new file" << endl;
    
    // Allocate inode for file
    int inodeNumber = inodeMgr->allocateInode(TYPE_FILE);
    
    if (inodeNumber == -1) {
        cerr << "[FileManager] ERROR: Failed to allocate inode for file" << endl;
        return -1;
    }
    
    cout << "[FileManager] File created with inode #" << inodeNumber << endl;
    return inodeNumber;
}

// Delete a file
bool FileManager::deleteFile(unsigned int inodeNumber) {
    cout << "[FileManager] Deleting file with inode #" << inodeNumber << endl;
    
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Read inode to get block pointers
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
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
        cerr << "[FileManager] ERROR: Failed to deallocate inode" << endl;
        return false;
    }
    
    cout << "[FileManager] File deleted successfully" << endl;
    return true;
}

// Write data to a file
bool FileManager::writeFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    cout << "[FileManager] Writing " << size << " bytes to file (inode #" << inodeNumber << ")" << endl;
    
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Read inode
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }
    
    // Calculate number of blocks needed
    unsigned int blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[FileManager] ERROR: File size exceeds maximum (only direct blocks supported)" << endl;
        return false;
    }
    
    // Allocate blocks if needed
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (inode.directBlocks[i] == 0) {
            int blockNum = blockMgr->allocateBlock();
            
            if (blockNum == -1) {
                cerr << "[FileManager] ERROR: Failed to allocate block" << endl;
                return false;
            }
            
            inode.directBlocks[i] = blockNum;
            inode.blockCount++;
        }
    }
    
    // Write data to blocks
    unsigned int bytesWritten = 0;
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        unsigned int bytesToWrite = (size - bytesWritten < BLOCK_SIZE) ? 
                                    (size - bytesWritten) : BLOCK_SIZE;
        
        if (!blockMgr->writeBlock(inode.directBlocks[i], data + bytesWritten, bytesToWrite)) {
            cerr << "[FileManager] ERROR: Failed to write to block" << endl;
            return false;
        }
        
        bytesWritten += bytesToWrite;
    }
    
    // Update inode
    inode.size = size;
    inode.modifiedTime = time(nullptr);
    
    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }
    
    cout << "[FileManager] Successfully wrote " << bytesWritten << " bytes" << endl;
    return true;
}

// Read data from a file
bool FileManager::readFile(unsigned int inodeNumber, char* buffer, unsigned int size) {
    cout << "[FileManager] Reading " << size << " bytes from file (inode #" << inodeNumber << ")" << endl;
    
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Read inode
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }
    
    // Limit size to file size
    if (size > inode.size) {
        size = inode.size;
    }
    
    // Calculate blocks to read
    unsigned int blocksToRead = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    // Read data from blocks
    unsigned int bytesRead = 0;
    for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == 0) {
            break;
        }
        
        unsigned int bytesToRead = (size - bytesRead < BLOCK_SIZE) ? 
                                   (size - bytesRead) : BLOCK_SIZE;
        
        if (!blockMgr->readBlock(inode.directBlocks[i], buffer + bytesRead, bytesToRead)) {
            cerr << "[FileManager] ERROR: Failed to read from block" << endl;
            return false;
        }
        
        bytesRead += bytesToRead;
    }
    
    cout << "[FileManager] Successfully read " << bytesRead << " bytes" << endl;
    return true;
}

// Append data to a file
bool FileManager::appendFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    cout << "[FileManager] Appending " << size << " bytes to file (inode #" << inodeNumber << ")" << endl;
    
    // For simplicity, we'll read existing data, then write combined data
    unsigned int currentSize = getFileSize(inodeNumber);
    unsigned int newSize = currentSize + size;
    
    // Allocate buffer for combined data
    char* combinedData = new char[newSize];
    
    // Read existing data
    if (currentSize > 0) {
        if (!readFile(inodeNumber, combinedData, currentSize)) {
            delete[] combinedData;
            return false;
        }
    }
    
    // Append new data
    memcpy(combinedData + currentSize, data, size);
    
    // Write combined data
    bool result = writeFile(inodeNumber, combinedData, newSize);
    
    delete[] combinedData;
    return result;
}

// Get file size
unsigned int FileManager::getFileSize(unsigned int inodeNumber) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        return 0;
    }
    
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        return 0;
    }
    
    return inode.size;
}

// Truncate file
bool FileManager::truncateFile(unsigned int inodeNumber) {
    cout << "[FileManager] Truncating file (inode #" << inodeNumber << ")" << endl;
    
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Read inode
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }
    
    // Deallocate all blocks
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != 0) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
            inode.directBlocks[i] = 0;
        }
    }
    
    // Update inode
    inode.size = 0;
    inode.blockCount = 0;
    inode.modifiedTime = time(nullptr);
    
    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }
    
    cout << "[FileManager] File truncated successfully" << endl;
    return true;
}

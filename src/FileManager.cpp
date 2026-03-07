#include "FileManager.h"
#include <cstring>

using namespace std;

// Constructor
FileManager::FileManager(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                         BlockManager* blockMgr, EncryptionManager* encryptMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr),
      encryptMgr(encryptMgr) {
    // cout << "[FileManager] Initialized" << endl;
    // if (encryptMgr && encryptMgr->isEnabled()) {
    //     cout << "[FileManager] Data encryption is ACTIVE" << endl;
    // }
}

// Destructor
FileManager::~FileManager() {
    // cout << "[FileManager] Destroyed" << endl;
}

// Create a new file
int FileManager::createFile() {
    // cout << "[FileManager] Creating new file" << endl;

    int inodeNumber = inodeMgr->allocateInode(TYPE_FILE);

    if (inodeNumber == -1) {
        cerr << "[FileManager] ERROR: Failed to allocate inode for file" << endl;
        return -1;
    }

    // cout << "[FileManager] File created with inode #" << inodeNumber << endl;
    return inodeNumber;
}

// Delete a file
bool FileManager::deleteFile(unsigned int inodeNumber) {
    // cout << "[FileManager] Deleting file with inode #" << inodeNumber << endl;

    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }

    // Deallocate all data blocks
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
        }
    }

    if (!inodeMgr->deallocateInode(inodeNumber)) {
        cerr << "[FileManager] ERROR: Failed to deallocate inode" << endl;
        return false;
    }

    // cout << "[FileManager] File deleted successfully" << endl;
    return true;
}

// Write data to a file.
// If encryption is active the data is encrypted block-by-block before
// being written to disk.  The original 'data' buffer is never modified.
bool FileManager::writeFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    // cout << "[FileManager] Writing " << size << " bytes to file (inode #"
    //      << inodeNumber << ")" << endl;

    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }

    unsigned int blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[FileManager] ERROR: File size exceeds maximum (only direct blocks supported)" << endl;
        return false;
    }

    // Allocate blocks if needed
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) {
            int blockNum = blockMgr->allocateBlock();
            if (blockNum == -1) {
                cerr << "[FileManager] ERROR: Failed to allocate block" << endl;
                return false;
            }
            inode.directBlocks[i] = blockNum;
            inode.blockCount++;
        }
    }

    // Write data to blocks, encrypting each block's worth of data
    unsigned int bytesWritten = 0;
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        unsigned int bytesToWrite = (size - bytesWritten < BLOCK_SIZE) ?
                                    (size - bytesWritten) : BLOCK_SIZE;

        // Work on a local copy so we don't corrupt the caller's buffer
        char blockBuf[BLOCK_SIZE];
        memset(blockBuf, 0, BLOCK_SIZE);
        memcpy(blockBuf, data + bytesWritten, bytesToWrite);

        // Encrypt before writing to disk
        if (encryptMgr && encryptMgr->isEnabled()) {
            encryptMgr->encrypt(blockBuf, bytesToWrite);
        }

        if (!blockMgr->writeBlock(inode.directBlocks[i], blockBuf, bytesToWrite)) {
            cerr << "[FileManager] ERROR: Failed to write to block" << endl;
            return false;
        }

        bytesWritten += bytesToWrite;
    }

    // Update inode metadata
    inode.size = size;
    inode.modifiedTime = time(nullptr);

    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }

    // cout << "[FileManager] Successfully wrote " << bytesWritten << " bytes" << endl;
    return true;
}

// Read data from a file.
// If encryption is active the raw bytes are decrypted after reading from disk.
bool FileManager::readFile(unsigned int inodeNumber, char* buffer, unsigned int size) {
    // cout << "[FileManager] Reading " << size << " bytes from file (inode #"
    //      << inodeNumber << ")" << endl;

    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }

    // Clamp to actual file size
    if (size > inode.size) {
        size = inode.size;
    }

    unsigned int blocksToRead = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    unsigned int bytesRead = 0;
    for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) {
            break;
        }

        unsigned int bytesToRead = (size - bytesRead < BLOCK_SIZE) ?
                                   (size - bytesRead) : BLOCK_SIZE;

        if (!blockMgr->readBlock(inode.directBlocks[i], buffer + bytesRead, bytesToRead)) {
            cerr << "[FileManager] ERROR: Failed to read from block" << endl;
            return false;
        }

        // Decrypt after reading from disk
        if (encryptMgr && encryptMgr->isEnabled()) {
            encryptMgr->decrypt(buffer + bytesRead, bytesToRead);
        }

        bytesRead += bytesToRead;
    }

    // cout << "[FileManager] Successfully read " << bytesRead << " bytes" << endl;
    return true;
}

// Append data to a file (read existing + write combined)
bool FileManager::appendFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    // cout << "[FileManager] Appending " << size << " bytes to file (inode #"
    //      << inodeNumber << ")" << endl;

    unsigned int currentSize = getFileSize(inodeNumber);
    unsigned int newSize = currentSize + size;

    char* combinedData = new char[newSize];

    if (currentSize > 0) {
        if (!readFile(inodeNumber, combinedData, currentSize)) {
            delete[] combinedData;
            return false;
        }
    }

    memcpy(combinedData + currentSize, data, size);

    bool result = writeFile(inodeNumber, combinedData, newSize);

    delete[] combinedData;
    return result;
}

// Get file size from inode
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

// Truncate file - set size to 0 and free all blocks
bool FileManager::truncateFile(unsigned int inodeNumber) {
    // cout << "[FileManager] Truncating file (inode #" << inodeNumber << ")" << endl;

    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to read inode" << endl;
        return false;
    }

    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
            inode.directBlocks[i] = NULL_BLOCK;
        }
    }

    inode.size = 0;
    inode.blockCount = 0;
    inode.modifiedTime = time(nullptr);

    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }

    // cout << "[FileManager] File truncated successfully" << endl;
    return true;
}

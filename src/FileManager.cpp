#include "FileManager.h"
#include <cstring>
#include <ctime>

using namespace std;

FileManager::FileManager(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                         BlockManager* blockMgr, EncryptionManager* encryptMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr),
      encryptMgr(encryptMgr) {}

FileManager::~FileManager() {}

// ── Private helpers ───────────────────────────────────────────────────────────

bool FileManager::readBlocks(const Inode& inode, char* buf, unsigned int size) {
    unsigned int blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    unsigned int bytesRead    = 0;

    for (unsigned int i = 0; i < blocksNeeded && i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) break;

        unsigned int bytesToRead = ((size - bytesRead) < BLOCK_SIZE)
                                   ? (size - bytesRead) : BLOCK_SIZE;

        if (!blockMgr->readBlock(inode.directBlocks[i], buf + bytesRead, bytesToRead)) {
            cerr << "[FileManager] ERROR: Block read failed" << endl;
            return false;
        }

        // Decrypt in-place after reading from disk
        if (encryptMgr && encryptMgr->isEnabled()) {
            encryptMgr->decrypt(buf + bytesRead, bytesToRead);
        }

        bytesRead += bytesToRead;
    }
    return true;
}

bool FileManager::writeBlocks(Inode& inode, const char* buf, unsigned int size) {
    unsigned int blocksNeeded = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[FileManager] ERROR: File too large (only direct blocks supported)" << endl;
        return false;
    }

    // Allocate any blocks not yet assigned
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) {
            int blk = blockMgr->allocateBlock();
            if (blk == -1) {
                cerr << "[FileManager] ERROR: Disk full — cannot allocate block" << endl;
                return false;
            }
            inode.directBlocks[i] = (unsigned int)blk;
            inode.blockCount++;
        }
    }

    // Write block by block, encrypting each chunk
    unsigned int bytesWritten = 0;
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        unsigned int bytesToWrite = ((size - bytesWritten) < BLOCK_SIZE)
                                   ? (size - bytesWritten) : BLOCK_SIZE;

        char blockBuf[BLOCK_SIZE];
        memset(blockBuf, 0, BLOCK_SIZE);
        memcpy(blockBuf, buf + bytesWritten, bytesToWrite);

        if (encryptMgr && encryptMgr->isEnabled()) {
            encryptMgr->encrypt(blockBuf, bytesToWrite);
        }

        if (!blockMgr->writeBlock(inode.directBlocks[i], blockBuf, bytesToWrite)) {
            cerr << "[FileManager] ERROR: Block write failed" << endl;
            return false;
        }

        bytesWritten += bytesToWrite;
    }
    return true;
}

// ── Public API ────────────────────────────────────────────────────────────────

int FileManager::createFile() {
    int inodeNumber = inodeMgr->allocateInode(TYPE_FILE);
    if (inodeNumber == -1) {
        cerr << "[FileManager] ERROR: Failed to allocate inode" << endl;
    }
    return inodeNumber;
}

bool FileManager::deleteFile(unsigned int inodeNumber) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode " << inodeNumber << " does not exist" << endl;
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
        }
    }

    if (!inodeMgr->deallocateInode(inodeNumber)) {
        cerr << "[FileManager] ERROR: Failed to deallocate inode" << endl;
        return false;
    }
    return true;
}

bool FileManager::writeFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode " << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) return false;

    if (!writeBlocks(inode, data, size)) return false;

    inode.size         = size;
    inode.modifiedTime = time(nullptr);

    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }
    return true;
}

bool FileManager::readFile(unsigned int inodeNumber, char* buffer, unsigned int size) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode " << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) return false;

    // Clamp to actual file size
    if (size > inode.size) size = inode.size;

    return readBlocks(inode, buffer, size);
}

bool FileManager::appendFile(unsigned int inodeNumber, const char* data, unsigned int size) {
    unsigned int currentSize = getFileSize(inodeNumber);
    unsigned int newSize     = currentSize + size;

    char* combined = new char[newSize];

    if (currentSize > 0) {
        if (!readFile(inodeNumber, combined, currentSize)) {
            delete[] combined;
            return false;
        }
    }

    memcpy(combined + currentSize, data, size);
    bool ok = writeFile(inodeNumber, combined, newSize);
    delete[] combined;
    return ok;
}

unsigned int FileManager::getFileSize(unsigned int inodeNumber) {
    if (!inodeMgr->inodeExists(inodeNumber)) return 0;
    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) return 0;
    return inode.size;
}

bool FileManager::truncateFile(unsigned int inodeNumber) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[FileManager] ERROR: Inode " << inodeNumber << " does not exist" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) return false;

    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
            inode.directBlocks[i] = NULL_BLOCK;
        }
    }

    inode.size         = 0;
    inode.blockCount   = 0;
    inode.modifiedTime = time(nullptr);

    if (!inodeMgr->writeInode(inodeNumber, inode)) {
        cerr << "[FileManager] ERROR: Failed to update inode" << endl;
        return false;
    }
    return true;
}

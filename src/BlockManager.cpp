#include "BlockManager.h"
#include <cstring>

using namespace std;

// Constructor
BlockManager::BlockManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* blockBmp)
    : disk(virtualDisk), superblockMgr(sbMgr), blockBitmap(blockBmp) {
    // NOTE: we do NOT cache the offset here — always read live from superblockMgr
    // so that managers created BEFORE formatFileSystem() still work correctly.
    // cout << "[BlockManager] Initialized" << endl;
}

// Destructor
BlockManager::~BlockManager() {
    // cout << "[BlockManager] Destroyed" << endl;
}

// Allocate a data block
int BlockManager::allocateBlock() {
    // cout << "[BlockManager] Allocating new data block" << endl;
    
    // Allocate from bitmap
    int blockNumber = blockBitmap->allocate();
    
    if (blockNumber == -1) {
        cerr << "[BlockManager] ERROR: Failed to allocate block from bitmap" << endl;
        return -1;
    }
    
    // Clear the block
    if (!clearBlock(blockNumber)) {
        cerr << "[BlockManager] ERROR: Failed to clear newly allocated block" << endl;
        blockBitmap->deallocate(blockNumber);
        return -1;
    }
    
    // Update superblock
    superblockMgr->decrementFreeBlocks();
    
    // cout << "[BlockManager] Allocated block #" << blockNumber << endl;
    return blockNumber;
}

// Deallocate a data block
bool BlockManager::deallocateBlock(unsigned int blockNumber) {
    // cout << "[BlockManager] Deallocating block #" << blockNumber << endl;
    
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block number " << blockNumber << " out of range" << endl;
        return false;
    }
    
    if (!isBlockAllocated(blockNumber)) {
        cerr << "[BlockManager] ERROR: Block #" << blockNumber << " already free" << endl;
        return false;
    }
    
    // Deallocate from bitmap
    if (!blockBitmap->deallocate(blockNumber)) {
        cerr << "[BlockManager] ERROR: Failed to deallocate block from bitmap" << endl;
        return false;
    }
    
    // Update superblock
    superblockMgr->incrementFreeBlocks();
    
    // cout << "[BlockManager] Deallocated block #" << blockNumber << endl;
    return true;
}

// Read data from a block
bool BlockManager::readBlock(unsigned int blockNumber, char* buffer, unsigned int size) {
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block number " << blockNumber << " out of range" << endl;
        return false;
    }
    
    if (size > BLOCK_SIZE) {
        cerr << "[BlockManager] ERROR: Read size " << size << " exceeds block size" << endl;
        return false;
    }
    
    unsigned int offset = getBlockOffset(blockNumber);
    
    // cout << "[BlockManager] Reading " << size << " bytes from block #" << blockNumber
    //      << " at offset " << offset << endl;
    
    return disk->readBlock(offset, buffer, size);
}

// Write data to a block
bool BlockManager::writeBlock(unsigned int blockNumber, const char* buffer, unsigned int size) {
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block number " << blockNumber << " out of range" << endl;
        return false;
    }
    
    if (size > BLOCK_SIZE) {
        cerr << "[BlockManager] ERROR: Write size " << size << " exceeds block size" << endl;
        return false;
    }
    
    unsigned int offset = getBlockOffset(blockNumber);
    
    // cout << "[BlockManager] Writing " << size << " bytes to block #" << blockNumber
    //      << " at offset " << offset << endl;
    
    return disk->writeBlock(offset, buffer, size);
}

// Clear a block
bool BlockManager::clearBlock(unsigned int blockNumber) {
    char zeroBuffer[BLOCK_SIZE];
    memset(zeroBuffer, 0, BLOCK_SIZE);
    
    // cout << "[BlockManager] Clearing block #" << blockNumber << endl;
    
    return writeBlock(blockNumber, zeroBuffer, BLOCK_SIZE);
}

// Check if block is allocated
bool BlockManager::isBlockAllocated(unsigned int blockNumber) const {
    if (blockNumber >= TOTAL_BLOCKS) {
        return false;
    }
    
    return blockBitmap->isUsed(blockNumber);
}

// Get block offset on disk — always compute live so it’s correct after format/load
unsigned int BlockManager::getBlockOffset(unsigned int blockNumber) const {
    return superblockMgr->getDataBlocksOffset() + (blockNumber * BLOCK_SIZE);
}

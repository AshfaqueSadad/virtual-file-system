#include "BlockManager.h"
#include <cstring>

using namespace std;

BlockManager::BlockManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr,
                           BitmapManager* blockBmp)
    : disk(virtualDisk), superblockMgr(sbMgr), blockBitmap(blockBmp) {}

BlockManager::~BlockManager() {}

int BlockManager::allocateBlock() {
    int blockNumber = blockBitmap->allocate();
    if (blockNumber == -1) {
        cerr << "[BlockManager] ERROR: No free blocks available" << endl;
        return -1;
    }
    if (!clearBlock((unsigned int)blockNumber)) {
        cerr << "[BlockManager] ERROR: Failed to zero new block" << endl;
        blockBitmap->deallocate((unsigned int)blockNumber);
        return -1;
    }
    superblockMgr->decrementFreeBlocks();
    return blockNumber;
}

bool BlockManager::deallocateBlock(unsigned int blockNumber) {
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block " << blockNumber << " out of range" << endl;
        return false;
    }
    if (!isBlockAllocated(blockNumber)) {
        cerr << "[BlockManager] ERROR: Block " << blockNumber << " already free" << endl;
        return false;
    }
    if (!blockBitmap->deallocate(blockNumber)) {
        cerr << "[BlockManager] ERROR: Bitmap deallocation failed" << endl;
        return false;
    }
    superblockMgr->incrementFreeBlocks();
    return true;
}

bool BlockManager::readBlock(unsigned int blockNumber, char* buffer, unsigned int size) {
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block " << blockNumber << " out of range" << endl;
        return false;
    }
    if (size > BLOCK_SIZE) {
        cerr << "[BlockManager] ERROR: Read size " << size << " exceeds block size" << endl;
        return false;
    }
    return disk->readBlock(getBlockOffset(blockNumber), buffer, size);
}

bool BlockManager::writeBlock(unsigned int blockNumber, const char* buffer, unsigned int size) {
    if (blockNumber >= TOTAL_BLOCKS) {
        cerr << "[BlockManager] ERROR: Block " << blockNumber << " out of range" << endl;
        return false;
    }
    if (size > BLOCK_SIZE) {
        cerr << "[BlockManager] ERROR: Write size " << size << " exceeds block size" << endl;
        return false;
    }
    return disk->writeBlock(getBlockOffset(blockNumber), buffer, size);
}

bool BlockManager::clearBlock(unsigned int blockNumber) {
    char zero[BLOCK_SIZE];
    memset(zero, 0, BLOCK_SIZE);
    return writeBlock(blockNumber, zero, BLOCK_SIZE);
}

bool BlockManager::isBlockAllocated(unsigned int blockNumber) const {
    if (blockNumber >= TOTAL_BLOCKS) return false;
    return blockBitmap->isUsed(blockNumber);
}

unsigned int BlockManager::getBlockOffset(unsigned int blockNumber) const {
    return superblockMgr->getDataBlocksOffset() + (blockNumber * BLOCK_SIZE);
}

unsigned int BlockManager::getFreeBlockCount() const {
    return blockBitmap->countFree();
}

unsigned int BlockManager::getUsedBlockCount() const {
    return blockBitmap->countUsed();
}

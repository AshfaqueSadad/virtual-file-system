#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "BitmapManager.h"
#include "SuperblockManager.h"
#include <iostream>
#include <vector>

class BlockManager {
private:
    VirtualDisk* disk;
    SuperblockManager* superblockMgr;
    BitmapManager* blockBitmap;
    unsigned int dataBlocksOffset;

public:
    // Constructor
    BlockManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* blockBmp);
    
    // Destructor
    ~BlockManager();
    
    // Allocate a data block (returns block number or -1 on failure)
    int allocateBlock();
    
    // Deallocate a data block
    bool deallocateBlock(unsigned int blockNumber);
    
    // Read data from a block
    bool readBlock(unsigned int blockNumber, char* buffer, unsigned int size);
    
    // Write data to a block
    bool writeBlock(unsigned int blockNumber, const char* buffer, unsigned int size);
    
    // Clear a block (fill with zeros)
    bool clearBlock(unsigned int blockNumber);
    
    // Check if block is allocated
    bool isBlockAllocated(unsigned int blockNumber) const;
    
    // Get block offset on disk
    unsigned int getBlockOffset(unsigned int blockNumber) const;
};

#endif // BLOCKMANAGER_H

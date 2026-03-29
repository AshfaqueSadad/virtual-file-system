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
    VirtualDisk*       disk;
    SuperblockManager* superblockMgr;
    BitmapManager*     blockBitmap;

public:
    BlockManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* blockBmp);
    ~BlockManager();

    // Allocate a data block — returns block number or -1 on failure
    int  allocateBlock();

    // Free a previously allocated data block
    bool deallocateBlock(unsigned int blockNumber);

    // Read up to 'size' bytes from a block into 'buffer'
    bool readBlock(unsigned int blockNumber, char* buffer, unsigned int size);

    // Write 'size' bytes from 'buffer' into a block
    bool writeBlock(unsigned int blockNumber, const char* buffer, unsigned int size);

    // Zero-fill a block
    bool clearBlock(unsigned int blockNumber);

    // Return true if block is currently allocated
    bool isBlockAllocated(unsigned int blockNumber) const;

    // Byte offset of this block on the disk image
    unsigned int getBlockOffset(unsigned int blockNumber) const;

    // Number of free data blocks remaining
    unsigned int getFreeBlockCount() const;

    // Number of data blocks currently in use
    unsigned int getUsedBlockCount() const;
};

#endif // BLOCKMANAGER_H

#ifndef INODEMANAGER_H
#define INODEMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "BitmapManager.h"
#include "SuperblockManager.h"
#include <iostream>

class InodeManager {
private:
    VirtualDisk*       disk;
    SuperblockManager* superblockMgr;
    BitmapManager*     inodeBitmap;

public:
    InodeManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* inodeBmp);
    ~InodeManager();

    // Allocate a new inode of the given type — returns inode number or -1
    int  allocateInode(InodeType type);

    // Free an allocated inode
    bool deallocateInode(unsigned int inodeNumber);

    // Read an inode from disk into 'inode'
    bool readInode(unsigned int inodeNumber, Inode& inode);

    // Persist an inode to disk
    bool writeInode(unsigned int inodeNumber, const Inode& inode);

    // Convenience: update only the size field of an inode
    bool updateInodeSize(unsigned int inodeNumber, uint32_t newSize);

    // Convenience: refresh modification and access timestamps
    bool updateInodeTimestamps(unsigned int inodeNumber);

    // Append a block pointer to the first free direct-block slot
    bool addBlockToInode(unsigned int inodeNumber, uint32_t blockNumber);

    // Return true if the inode is allocated (bit set in bitmap)
    bool inodeExists(unsigned int inodeNumber) const;

    // Byte offset of inode 'inodeNumber' within the inode table
    unsigned int getInodeOffset(unsigned int inodeNumber) const;

    // Number of free inodes remaining
    unsigned int getFreeInodeCount() const;

    // Number of inodes currently in use
    unsigned int getUsedInodeCount() const;

    // Dump inode metadata to stdout (debug helper)
    void printInode(unsigned int inodeNumber) const;
};

#endif // INODEMANAGER_H

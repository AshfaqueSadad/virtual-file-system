#ifndef INODEMANAGER_H
#define INODEMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "BitmapManager.h"
#include "SuperblockManager.h"
#include <iostream>

class InodeManager {
private:
    VirtualDisk* disk;
    SuperblockManager* superblockMgr;
    BitmapManager* inodeBitmap;
    unsigned int inodeTableOffset;

public:
    // Constructor
    InodeManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* inodeBmp);
    
    // Destructor
    ~InodeManager();
    
    // Allocate a new inode (returns inode number or -1 on failure)
    int allocateInode(InodeType type);
    
    // Deallocate an inode
    bool deallocateInode(unsigned int inodeNumber);
    
    // Read an inode from disk
    bool readInode(unsigned int inodeNumber, Inode& inode);
    
    // Write an inode to disk
    bool writeInode(unsigned int inodeNumber, const Inode& inode);
    
    // Update inode size
    bool updateInodeSize(unsigned int inodeNumber, uint32_t newSize);
    
    // Update inode timestamps
    bool updateInodeTimestamps(unsigned int inodeNumber);
    
    // Add block pointer to inode
    bool addBlockToInode(unsigned int inodeNumber, uint32_t blockNumber);
    
    // Check if inode exists
    bool inodeExists(unsigned int inodeNumber) const;
    
    // Print inode information (for debugging)
    void printInode(unsigned int inodeNumber) const;
};

#endif // INODEMANAGER_H

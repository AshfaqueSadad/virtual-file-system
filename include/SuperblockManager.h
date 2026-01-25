#ifndef SUPERBLOCKMANAGER_H
#define SUPERBLOCKMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include <iostream>

class SuperblockManager {
private:
    VirtualDisk* disk;
    Superblock superblock;
    bool isLoaded;

public:
    // Constructor
    SuperblockManager(VirtualDisk* virtualDisk);
    
    // Destructor
    ~SuperblockManager();
    
    // Initialize a new superblock with default values
    bool initialize();
    
    // Load superblock from disk
    bool load();
    
    // Save superblock to disk
    bool save();
    
    // Validate superblock (check magic number)
    bool validate() const;
    
    // Getters
    uint32_t getTotalBlocks() const;
    uint32_t getTotalInodes() const;
    uint32_t getFreeBlocks() const;
    uint32_t getFreeInodes() const;
    uint32_t getBlockSize() const;
    uint32_t getInodeSize() const;
    uint32_t getInodeBitmapOffset() const;
    uint32_t getBlockBitmapOffset() const;
    uint32_t getInodeTableOffset() const;
    uint32_t getDataBlocksOffset() const;
    uint32_t getRootInodeNumber() const;
    
    // Setters
    void setFreeBlocks(uint32_t count);
    void setFreeInodes(uint32_t count);
    void decrementFreeBlocks();
    void incrementFreeBlocks();
    void decrementFreeInodes();
    void incrementFreeInodes();
    
    // Print superblock information
    void print() const;
};

#endif // SUPERBLOCKMANAGER_H

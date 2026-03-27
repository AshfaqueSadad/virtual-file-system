#ifndef SUPERBLOCKMANAGER_H
#define SUPERBLOCKMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include <iostream>

class SuperblockManager {
private:
    VirtualDisk* disk;
    Superblock   superblock;
    bool         isLoaded;

public:
    SuperblockManager(VirtualDisk* virtualDisk);
    ~SuperblockManager();

    // Initialise a fresh superblock and save it to disk
    bool initialize();

    // Load the superblock from disk
    bool load();

    // Persist the in-memory superblock to disk
    bool save();

    // Return true if the magic number matches EXT2_MAGIC
    bool validate() const;

    // ── Getters ──────────────────────────────────────────────────────────────
    uint32_t getTotalBlocks()       const;
    uint32_t getTotalInodes()       const;
    uint32_t getFreeBlocks()        const;
    uint32_t getFreeInodes()        const;
    uint32_t getBlockSize()         const;
    uint32_t getInodeSize()         const;
    uint32_t getInodeBitmapOffset() const;
    uint32_t getBlockBitmapOffset() const;
    uint32_t getInodeTableOffset()  const;
    uint32_t getDataBlocksOffset()  const;
    uint32_t getRootInodeNumber()   const;

    // ── Free-count mutators ───────────────────────────────────────────────────
    void setFreeBlocks(uint32_t count);
    void setFreeInodes(uint32_t count);
    void decrementFreeBlocks();
    void incrementFreeBlocks();
    void decrementFreeInodes();
    void incrementFreeInodes();

    // ── Offset setters (used during format) ──────────────────────────────────
    void setInodeBitmapOffset(uint32_t offset);
    void setBlockBitmapOffset(uint32_t offset);
    void setInodeTableOffset(uint32_t offset);
    void setDataBlocksOffset(uint32_t offset);

    // Dump superblock fields to stdout (debug helper)
    void print() const;
};

#endif // SUPERBLOCKMANAGER_H

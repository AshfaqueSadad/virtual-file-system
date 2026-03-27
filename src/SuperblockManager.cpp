#include "SuperblockManager.h"
#include <cstring>

using namespace std;

SuperblockManager::SuperblockManager(VirtualDisk* virtualDisk)
    : disk(virtualDisk), isLoaded(false) {}

SuperblockManager::~SuperblockManager() {}

bool SuperblockManager::initialize() {
    superblock = Superblock();

    // Disk layout:
    //   Block 0            → Superblock (1 block)
    //   Block 1            → Inode bitmap (1 block)
    //   Blocks 2–3         → Block bitmap (2 blocks)
    //   Blocks 4–131       → Inode table  (128 blocks = 1024 × 128 B)
    //   Block 132 onwards  → Data blocks

    unsigned int off = BLOCK_SIZE;           // byte offset after superblock

    superblock.inodeBitmapOffset = off;
    off += BLOCK_SIZE;

    superblock.blockBitmapOffset = off;
    off += 2 * BLOCK_SIZE;

    superblock.inodeTableOffset  = off;
    off += TOTAL_INODES * INODE_SIZE;

    superblock.dataBlocksOffset  = off;

    superblock.freeBlocks      = TOTAL_BLOCKS;
    superblock.freeInodes      = TOTAL_INODES;
    superblock.rootInodeNumber = 0;

    isLoaded = true;
    return save();
}

bool SuperblockManager::load() {
    char buf[BLOCK_SIZE];
    if (!disk->readBlock(0, buf, BLOCK_SIZE)) {
        cerr << "[SuperblockManager] ERROR: Cannot read superblock" << endl;
        return false;
    }
    memcpy(&superblock, buf, sizeof(Superblock));
    if (!validate()) {
        cerr << "[SuperblockManager] ERROR: Magic number mismatch" << endl;
        return false;
    }
    isLoaded = true;
    return true;
}

bool SuperblockManager::save() {
    char buf[BLOCK_SIZE];
    memset(buf, 0, BLOCK_SIZE);
    memcpy(buf, &superblock, sizeof(Superblock));
    if (!disk->writeBlock(0, buf, BLOCK_SIZE)) {
        cerr << "[SuperblockManager] ERROR: Cannot write superblock" << endl;
        return false;
    }
    disk->flush();
    return true;
}

bool SuperblockManager::validate() const {
    if (superblock.magic != EXT2_MAGIC) {
        cerr << "[SuperblockManager] Invalid magic: 0x" << hex << superblock.magic
             << " (expected 0x" << EXT2_MAGIC << ")" << dec << endl;
        return false;
    }
    return true;
}

// ── Getters ──────────────────────────────────────────────────────────────────
uint32_t SuperblockManager::getTotalBlocks()       const { return superblock.totalBlocks; }
uint32_t SuperblockManager::getTotalInodes()       const { return superblock.totalInodes; }
uint32_t SuperblockManager::getFreeBlocks()        const { return superblock.freeBlocks; }
uint32_t SuperblockManager::getFreeInodes()        const { return superblock.freeInodes; }
uint32_t SuperblockManager::getBlockSize()         const { return superblock.blockSize; }
uint32_t SuperblockManager::getInodeSize()         const { return superblock.inodeSize; }
uint32_t SuperblockManager::getInodeBitmapOffset() const { return superblock.inodeBitmapOffset; }
uint32_t SuperblockManager::getBlockBitmapOffset() const { return superblock.blockBitmapOffset; }
uint32_t SuperblockManager::getInodeTableOffset()  const { return superblock.inodeTableOffset; }
uint32_t SuperblockManager::getDataBlocksOffset()  const { return superblock.dataBlocksOffset; }
uint32_t SuperblockManager::getRootInodeNumber()   const { return superblock.rootInodeNumber; }

// ── Free-count mutators ───────────────────────────────────────────────────────
void SuperblockManager::setFreeBlocks(uint32_t c) { superblock.freeBlocks = c; save(); }
void SuperblockManager::setFreeInodes(uint32_t c) { superblock.freeInodes = c; save(); }

void SuperblockManager::decrementFreeBlocks() {
    if (superblock.freeBlocks > 0) { superblock.freeBlocks--; save(); }
}
void SuperblockManager::incrementFreeBlocks() { superblock.freeBlocks++; save(); }

void SuperblockManager::decrementFreeInodes() {
    if (superblock.freeInodes > 0) { superblock.freeInodes--; save(); }
}
void SuperblockManager::incrementFreeInodes() { superblock.freeInodes++; save(); }

// ── Offset setters ────────────────────────────────────────────────────────────
void SuperblockManager::setInodeBitmapOffset(uint32_t o) { superblock.inodeBitmapOffset = o; save(); }
void SuperblockManager::setBlockBitmapOffset(uint32_t o) { superblock.blockBitmapOffset = o; save(); }
void SuperblockManager::setInodeTableOffset(uint32_t o)  { superblock.inodeTableOffset  = o; save(); }
void SuperblockManager::setDataBlocksOffset(uint32_t o)  { superblock.dataBlocksOffset  = o; save(); }

// ── Debug print ───────────────────────────────────────────────────────────────
void SuperblockManager::print() const {
    cout << "\n=== Superblock ===" << endl;
    cout << "Magic:              0x" << hex << superblock.magic << dec << endl;
    cout << "Total Blocks:       " << superblock.totalBlocks << endl;
    cout << "Free Blocks:        " << superblock.freeBlocks  << endl;
    cout << "Total Inodes:       " << superblock.totalInodes << endl;
    cout << "Free Inodes:        " << superblock.freeInodes  << endl;
    cout << "Block Size:         " << superblock.blockSize   << " bytes" << endl;
    cout << "Inode Bitmap Offset:" << superblock.inodeBitmapOffset << endl;
    cout << "Block Bitmap Offset:" << superblock.blockBitmapOffset << endl;
    cout << "Inode Table Offset: " << superblock.inodeTableOffset  << endl;
    cout << "Data Blocks Offset: " << superblock.dataBlocksOffset  << endl;
    cout << "==================\n" << endl;
}

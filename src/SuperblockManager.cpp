#include "SuperblockManager.h"
#include <cstring>

using namespace std;

// Constructor
SuperblockManager::SuperblockManager(VirtualDisk* virtualDisk)
    : disk(virtualDisk), isLoaded(false) {
    cout << "[SuperblockManager] Initialized" << endl;
}

// Destructor
SuperblockManager::~SuperblockManager() {
    cout << "[SuperblockManager] Destroyed" << endl;
}

// Initialize superblock
bool SuperblockManager::initialize() {
    cout << "[SuperblockManager] Initializing new superblock" << endl;
    
    // Create default superblock
    superblock = Superblock();
    
    // Calculate offsets
    // Layout: [Superblock(1 block)] [Inode Bitmap] [Block Bitmap] [Inode Table] [Data Blocks]
    
    unsigned int currentOffset = BLOCK_SIZE;  // After superblock
    
    // Inode bitmap: 1024 inodes / 8 bits per byte = 128 bytes (1 block to be safe)
    superblock.inodeBitmapOffset = currentOffset;
    currentOffset += BLOCK_SIZE;
    
    // Block bitmap: 10240 blocks / 8 bits per byte = 1280 bytes (2 blocks to be safe)
    superblock.blockBitmapOffset = currentOffset;
    currentOffset += 2 * BLOCK_SIZE;
    
    // Inode table: 1024 inodes * 128 bytes = 131072 bytes (128 blocks)
    superblock.inodeTableOffset = currentOffset;
    currentOffset += (TOTAL_INODES * INODE_SIZE);
    
    // Data blocks: remaining space
    superblock.dataBlocksOffset = currentOffset;
    
    // Initially all blocks and inodes are free
    superblock.freeBlocks = TOTAL_BLOCKS;
    superblock.freeInodes = TOTAL_INODES;
    
    // Root inode is 0
    superblock.rootInodeNumber = 0;
    
    isLoaded = true;
    
    cout << "[SuperblockManager] Superblock initialized" << endl;
    cout << "  Inode Bitmap Offset: " << superblock.inodeBitmapOffset << endl;
    cout << "  Block Bitmap Offset: " << superblock.blockBitmapOffset << endl;
    cout << "  Inode Table Offset: " << superblock.inodeTableOffset << endl;
    cout << "  Data Blocks Offset: " << superblock.dataBlocksOffset << endl;
    
    // Save to disk
    return save();
}

// Load superblock from disk
bool SuperblockManager::load() {
    cout << "[SuperblockManager] Loading superblock from disk" << endl;
    
    char buffer[BLOCK_SIZE];
    
    if (!disk->readBlock(0, buffer, BLOCK_SIZE)) {
        cerr << "[SuperblockManager] ERROR: Failed to read superblock from disk" << endl;
        return false;
    }
    
    // Copy buffer to superblock structure
    memcpy(&superblock, buffer, sizeof(Superblock));
    
    if (!validate()) {
        cerr << "[SuperblockManager] ERROR: Invalid superblock (magic number mismatch)" << endl;
        return false;
    }
    
    isLoaded = true;
    cout << "[SuperblockManager] Superblock loaded successfully" << endl;
    return true;
}

// Save superblock to disk
bool SuperblockManager::save() {
    cout << "[SuperblockManager] Saving superblock to disk" << endl;
    
    char buffer[BLOCK_SIZE];
    memset(buffer, 0, BLOCK_SIZE);
    
    // Copy superblock to buffer
    memcpy(buffer, &superblock, sizeof(Superblock));
    
    if (!disk->writeBlock(0, buffer, BLOCK_SIZE)) {
        cerr << "[SuperblockManager] ERROR: Failed to write superblock to disk" << endl;
        return false;
    }
    
    disk->flush();
    cout << "[SuperblockManager] Superblock saved successfully" << endl;
    return true;
}

// Validate superblock
bool SuperblockManager::validate() const {
    if (superblock.magic != EXT2_MAGIC) {
        cerr << "[SuperblockManager] Invalid magic number: " << hex << superblock.magic 
             << " (expected: " << EXT2_MAGIC << ")" << dec << endl;
        return false;
    }
    return true;
}

// Getters
uint32_t SuperblockManager::getTotalBlocks() const { return superblock.totalBlocks; }
uint32_t SuperblockManager::getTotalInodes() const { return superblock.totalInodes; }
uint32_t SuperblockManager::getFreeBlocks() const { return superblock.freeBlocks; }
uint32_t SuperblockManager::getFreeInodes() const { return superblock.freeInodes; }
uint32_t SuperblockManager::getBlockSize() const { return superblock.blockSize; }
uint32_t SuperblockManager::getInodeSize() const { return superblock.inodeSize; }
uint32_t SuperblockManager::getInodeBitmapOffset() const { return superblock.inodeBitmapOffset; }
uint32_t SuperblockManager::getBlockBitmapOffset() const { return superblock.blockBitmapOffset; }
uint32_t SuperblockManager::getInodeTableOffset() const { return superblock.inodeTableOffset; }
uint32_t SuperblockManager::getDataBlocksOffset() const { return superblock.dataBlocksOffset; }
uint32_t SuperblockManager::getRootInodeNumber() const { return superblock.rootInodeNumber; }

// Setters
void SuperblockManager::setFreeBlocks(uint32_t count) {
    superblock.freeBlocks = count;
    save();
}

void SuperblockManager::setFreeInodes(uint32_t count) {
    superblock.freeInodes = count;
    save();
}

void SuperblockManager::decrementFreeBlocks() {
    if (superblock.freeBlocks > 0) {
        superblock.freeBlocks--;
        save();
    }
}

void SuperblockManager::incrementFreeBlocks() {
    superblock.freeBlocks++;
    save();
}

void SuperblockManager::decrementFreeInodes() {
    if (superblock.freeInodes > 0) {
        superblock.freeInodes--;
        save();
    }
}

void SuperblockManager::incrementFreeInodes() {
    superblock.freeInodes++;
    save();
}

// Print superblock info
void SuperblockManager::print() const {
    cout << "\n=== Superblock Information ===" << endl;
    cout << "Magic Number: 0x" << hex << superblock.magic << dec << endl;
    cout << "Total Blocks: " << superblock.totalBlocks << endl;
    cout << "Free Blocks: " << superblock.freeBlocks << endl;
    cout << "Total Inodes: " << superblock.totalInodes << endl;
    cout << "Free Inodes: " << superblock.freeInodes << endl;
    cout << "Block Size: " << superblock.blockSize << " bytes" << endl;
    cout << "Inode Size: " << superblock.inodeSize << " bytes" << endl;
    cout << "Inode Bitmap Offset: " << superblock.inodeBitmapOffset << endl;
    cout << "Block Bitmap Offset: " << superblock.blockBitmapOffset << endl;
    cout << "Inode Table Offset: " << superblock.inodeTableOffset << endl;
    cout << "Data Blocks Offset: " << superblock.dataBlocksOffset << endl;
    cout << "Root Inode Number: " << superblock.rootInodeNumber << endl;
    cout << "==============================\n" << endl;
}

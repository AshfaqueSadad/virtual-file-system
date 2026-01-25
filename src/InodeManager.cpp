#include "InodeManager.h"
#include <cstring>
#include <ctime>

using namespace std;

// Constructor
InodeManager::InodeManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr, BitmapManager* inodeBmp)
    : disk(virtualDisk), superblockMgr(sbMgr), inodeBitmap(inodeBmp) {
    
    inodeTableOffset = sbMgr->getInodeTableOffset();
    cout << "[InodeManager] Initialized with inode table at offset " << inodeTableOffset << endl;
}

// Destructor
InodeManager::~InodeManager() {
    cout << "[InodeManager] Destroyed" << endl;
}

// Allocate a new inode
int InodeManager::allocateInode(InodeType type) {
    cout << "[InodeManager] Allocating new inode of type " << type << endl;
    
    // Allocate from bitmap
    int inodeNumber = inodeBitmap->allocate();
    
    if (inodeNumber == -1) {
        cerr << "[InodeManager] ERROR: Failed to allocate inode from bitmap" << endl;
        return -1;
    }
    
    // Create new inode
    Inode inode;
    inode.type = type;
    inode.permissions = 0755;  // Default permissions
    inode.size = 0;
    inode.blockCount = 0;
    inode.createdTime = time(nullptr);
    inode.modifiedTime = inode.createdTime;
    inode.accessedTime = inode.createdTime;
    
    // Initialize block pointers to 0
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        inode.directBlocks[i] = 0;
    }
    inode.singleIndirect = 0;
    inode.doubleIndirect = 0;
    
    // Write inode to disk
    if (!writeInode(inodeNumber, inode)) {
        cerr << "[InodeManager] ERROR: Failed to write new inode to disk" << endl;
        inodeBitmap->deallocate(inodeNumber);
        return -1;
    }
    
    // Update superblock
    superblockMgr->decrementFreeInodes();
    
    cout << "[InodeManager] Allocated inode #" << inodeNumber << endl;
    return inodeNumber;
}

// Deallocate an inode
bool InodeManager::deallocateInode(unsigned int inodeNumber) {
    cout << "[InodeManager] Deallocating inode #" << inodeNumber << endl;
    
    if (!inodeExists(inodeNumber)) {
        cerr << "[InodeManager] ERROR: Inode #" << inodeNumber << " does not exist" << endl;
        return false;
    }
    
    // Deallocate from bitmap
    if (!inodeBitmap->deallocate(inodeNumber)) {
        cerr << "[InodeManager] ERROR: Failed to deallocate inode from bitmap" << endl;
        return false;
    }
    
    // Update superblock
    superblockMgr->incrementFreeInodes();
    
    cout << "[InodeManager] Deallocated inode #" << inodeNumber << endl;
    return true;
}

// Read inode from disk
bool InodeManager::readInode(unsigned int inodeNumber, Inode& inode) {
    if (inodeNumber >= TOTAL_INODES) {
        cerr << "[InodeManager] ERROR: Inode number " << inodeNumber << " out of range" << endl;
        return false;
    }
    
    // Calculate offset in inode table
    unsigned int offset = inodeTableOffset + (inodeNumber * INODE_SIZE);
    
    cout << "[InodeManager] Reading inode #" << inodeNumber << " from offset " << offset << endl;
    
    char buffer[INODE_SIZE];
    
    if (!disk->readBlock(offset, buffer, INODE_SIZE)) {
        cerr << "[InodeManager] ERROR: Failed to read inode from disk" << endl;
        return false;
    }
    
    // Copy buffer to inode structure
    memcpy(&inode, buffer, sizeof(Inode));
    
    return true;
}

// Write inode to disk
bool InodeManager::writeInode(unsigned int inodeNumber, const Inode& inode) {
    if (inodeNumber >= TOTAL_INODES) {
        cerr << "[InodeManager] ERROR: Inode number " << inodeNumber << " out of range" << endl;
        return false;
    }
    
    // Calculate offset in inode table
    unsigned int offset = inodeTableOffset + (inodeNumber * INODE_SIZE);
    
    cout << "[InodeManager] Writing inode #" << inodeNumber << " to offset " << offset << endl;
    
    char buffer[INODE_SIZE];
    memset(buffer, 0, INODE_SIZE);
    
    // Copy inode structure to buffer
    memcpy(buffer, &inode, sizeof(Inode));
    
    if (!disk->writeBlock(offset, buffer, INODE_SIZE)) {
        cerr << "[InodeManager] ERROR: Failed to write inode to disk" << endl;
        return false;
    }
    
    disk->flush();
    return true;
}

// Update inode size
bool InodeManager::updateInodeSize(unsigned int inodeNumber, uint32_t newSize) {
    Inode inode;
    
    if (!readInode(inodeNumber, inode)) {
        return false;
    }
    
    inode.size = newSize;
    inode.modifiedTime = time(nullptr);
    
    return writeInode(inodeNumber, inode);
}

// Update inode timestamps
bool InodeManager::updateInodeTimestamps(unsigned int inodeNumber) {
    Inode inode;
    
    if (!readInode(inodeNumber, inode)) {
        return false;
    }
    
    inode.modifiedTime = time(nullptr);
    inode.accessedTime = inode.modifiedTime;
    
    return writeInode(inodeNumber, inode);
}

// Add block pointer to inode
bool InodeManager::addBlockToInode(unsigned int inodeNumber, uint32_t blockNumber) {
    Inode inode;
    
    if (!readInode(inodeNumber, inode)) {
        return false;
    }
    
    // Find first empty direct block pointer
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == 0) {
            inode.directBlocks[i] = blockNumber;
            inode.blockCount++;
            inode.modifiedTime = time(nullptr);
            
            cout << "[InodeManager] Added block " << blockNumber 
                 << " to inode #" << inodeNumber << " at position " << i << endl;
            
            return writeInode(inodeNumber, inode);
        }
    }
    
    cerr << "[InodeManager] ERROR: No free direct block pointers in inode #" 
         << inodeNumber << endl;
    return false;
}

// Check if inode exists
bool InodeManager::inodeExists(unsigned int inodeNumber) const {
    if (inodeNumber >= TOTAL_INODES) {
        return false;
    }
    
    return inodeBitmap->isUsed(inodeNumber);
}

// Print inode information
void InodeManager::printInode(unsigned int inodeNumber) const {
    Inode inode;
    
    if (!const_cast<InodeManager*>(this)->readInode(inodeNumber, inode)) {
        cerr << "[InodeManager] Cannot print inode #" << inodeNumber << endl;
        return;
    }
    
    cout << "\n=== Inode #" << inodeNumber << " ===" << endl;
    cout << "Type: " << (inode.type == TYPE_FILE ? "File" : "Directory") << endl;
    cout << "Size: " << inode.size << " bytes" << endl;
    cout << "Block Count: " << inode.blockCount << endl;
    cout << "Created: " << ctime(&inode.createdTime);
    cout << "Modified: " << ctime(&inode.modifiedTime);
    cout << "Direct Blocks: ";
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != 0) {
            cout << inode.directBlocks[i] << " ";
        }
    }
    cout << "\n========================\n" << endl;
}

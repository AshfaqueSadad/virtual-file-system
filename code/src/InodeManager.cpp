#include "InodeManager.h"
#include <cstring>
#include <ctime>

using namespace std;

InodeManager::InodeManager(VirtualDisk* virtualDisk, SuperblockManager* sbMgr,
                           BitmapManager* inodeBmp)
    : disk(virtualDisk), superblockMgr(sbMgr), inodeBitmap(inodeBmp) {}

InodeManager::~InodeManager() {}

int InodeManager::allocateInode(InodeType type) {
    int inodeNumber = inodeBitmap->allocate();
    if (inodeNumber == -1) {
        cerr << "[InodeManager] ERROR: No free inodes available" << endl;
        return -1;
    }

    Inode inode;
    inode.type          = type;
    inode.permissions   = 0755;
    inode.size          = 0;
    inode.blockCount    = 0;
    inode.createdTime   = time(nullptr);
    inode.modifiedTime  = inode.createdTime;
    inode.accessedTime  = inode.createdTime;
    inode.singleIndirect = NULL_BLOCK;
    inode.doubleIndirect = NULL_BLOCK;
    for (int i = 0; i < DIRECT_BLOCKS; i++) inode.directBlocks[i] = NULL_BLOCK;

    if (!writeInode((unsigned int)inodeNumber, inode)) {
        cerr << "[InodeManager] ERROR: Failed to persist new inode" << endl;
        inodeBitmap->deallocate((unsigned int)inodeNumber);
        return -1;
    }

    superblockMgr->decrementFreeInodes();
    return inodeNumber;
}

bool InodeManager::deallocateInode(unsigned int inodeNumber) {
    if (!inodeExists(inodeNumber)) {
        cerr << "[InodeManager] ERROR: Inode " << inodeNumber << " does not exist" << endl;
        return false;
    }
    if (!inodeBitmap->deallocate(inodeNumber)) {
        cerr << "[InodeManager] ERROR: Bitmap deallocation failed" << endl;
        return false;
    }
    superblockMgr->incrementFreeInodes();
    return true;
}

bool InodeManager::readInode(unsigned int inodeNumber, Inode& inode) {
    if (inodeNumber >= TOTAL_INODES) {
        cerr << "[InodeManager] ERROR: Inode number " << inodeNumber << " out of range" << endl;
        return false;
    }
    char buf[INODE_SIZE];
    if (!disk->readBlock(getInodeOffset(inodeNumber), buf, INODE_SIZE)) {
        cerr << "[InodeManager] ERROR: Failed to read inode " << inodeNumber << endl;
        return false;
    }
    memcpy(&inode, buf, sizeof(Inode));
    return true;
}

bool InodeManager::writeInode(unsigned int inodeNumber, const Inode& inode) {
    if (inodeNumber >= TOTAL_INODES) {
        cerr << "[InodeManager] ERROR: Inode number " << inodeNumber << " out of range" << endl;
        return false;
    }
    char buf[INODE_SIZE];
    memset(buf, 0, INODE_SIZE);
    memcpy(buf, &inode, sizeof(Inode));
    if (!disk->writeBlock(getInodeOffset(inodeNumber), buf, INODE_SIZE)) {
        cerr << "[InodeManager] ERROR: Failed to write inode " << inodeNumber << endl;
        return false;
    }
    disk->flush();
    return true;
}

bool InodeManager::updateInodeSize(unsigned int inodeNumber, uint32_t newSize) {
    Inode inode;
    if (!readInode(inodeNumber, inode)) return false;
    inode.size         = newSize;
    inode.modifiedTime = time(nullptr);
    return writeInode(inodeNumber, inode);
}

bool InodeManager::updateInodeTimestamps(unsigned int inodeNumber) {
    Inode inode;
    if (!readInode(inodeNumber, inode)) return false;
    inode.modifiedTime = time(nullptr);
    inode.accessedTime = inode.modifiedTime;
    return writeInode(inodeNumber, inode);
}

bool InodeManager::addBlockToInode(unsigned int inodeNumber, uint32_t blockNumber) {
    Inode inode;
    if (!readInode(inodeNumber, inode)) return false;
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) {
            inode.directBlocks[i] = blockNumber;
            inode.blockCount++;
            inode.modifiedTime = time(nullptr);
            return writeInode(inodeNumber, inode);
        }
    }
    cerr << "[InodeManager] ERROR: All direct block slots used in inode " << inodeNumber << endl;
    return false;
}

bool InodeManager::inodeExists(unsigned int inodeNumber) const {
    if (inodeNumber >= TOTAL_INODES) return false;
    return inodeBitmap->isUsed(inodeNumber);
}

unsigned int InodeManager::getInodeOffset(unsigned int inodeNumber) const {
    return superblockMgr->getInodeTableOffset() + (inodeNumber * INODE_SIZE);
}

unsigned int InodeManager::getFreeInodeCount() const {
    return inodeBitmap->countFree();
}

unsigned int InodeManager::getUsedInodeCount() const {
    return inodeBitmap->countUsed();
}

void InodeManager::printInode(unsigned int inodeNumber) const {
    Inode inode;
    if (!const_cast<InodeManager*>(this)->readInode(inodeNumber, inode)) {
        cerr << "[InodeManager] Cannot print inode " << inodeNumber << endl;
        return;
    }
    cout << "\n=== Inode #" << inodeNumber << " ===" << endl;
    cout << "Type:        " << (inode.type == TYPE_FILE ? "File" : "Directory") << endl;
    cout << "Size:        " << inode.size << " bytes" << endl;
    cout << "Block Count: " << inode.blockCount << endl;
    cout << "Created:     " << ctime(&inode.createdTime);
    cout << "Modified:    " << ctime(&inode.modifiedTime);
    cout << "Direct Blocks: ";
    for (int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) cout << inode.directBlocks[i] << " ";
    }
    cout << "\n========================\n" << endl;
}

#include "DirectoryHandler.h"
#include <cstring>
#include <ctime>

using namespace std;

DirectoryHandler::DirectoryHandler(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                                   BlockManager* blockMgr, FileManager* fileMgr)
    : disk(virtualDisk), inodeMgr(inodeMgr), blockMgr(blockMgr), fileMgr(fileMgr) {}

DirectoryHandler::~DirectoryHandler() {}

// ── Private helpers ───────────────────────────────────────────────────────────

char* DirectoryHandler::loadEntries(unsigned int dirInode, unsigned int& outSize) {
    Inode inode;
    if (!inodeMgr->readInode(dirInode, inode) || inode.size == 0) {
        outSize = 0;
        return nullptr;
    }

    outSize         = inode.size;
    char* buf       = new char[outSize];
    memset(buf, 0, outSize);

    unsigned int blocksToRead = (outSize + BLOCK_SIZE - 1) / BLOCK_SIZE;
    unsigned int bytesRead    = 0;

    for (unsigned int i = 0; i < blocksToRead && i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) break;
        unsigned int chunk = ((outSize - bytesRead) < BLOCK_SIZE)
                             ? (outSize - bytesRead) : BLOCK_SIZE;
        blockMgr->readBlock(inode.directBlocks[i], buf + bytesRead, chunk);
        bytesRead += chunk;
    }
    return buf;
}

bool DirectoryHandler::flushEntries(unsigned int dirInode, const char* buf,
                                    unsigned int newSize, Inode& inode) {
    unsigned int blocksNeeded = (newSize > 0) ? (newSize + BLOCK_SIZE - 1) / BLOCK_SIZE : 0;

    if (blocksNeeded > DIRECT_BLOCKS) {
        cerr << "[DirectoryHandler] ERROR: Directory too large" << endl;
        return false;
    }

    // Allocate any new blocks required
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        if (inode.directBlocks[i] == NULL_BLOCK) {
            int blk = blockMgr->allocateBlock();
            if (blk == -1) {
                cerr << "[DirectoryHandler] ERROR: Disk full" << endl;
                return false;
            }
            inode.directBlocks[i] = (unsigned int)blk;
            inode.blockCount++;
        }
    }

    // Write data blocks
    unsigned int bytesWritten = 0;
    for (unsigned int i = 0; i < blocksNeeded; i++) {
        unsigned int chunk = ((newSize - bytesWritten) < BLOCK_SIZE)
                             ? (newSize - bytesWritten) : BLOCK_SIZE;
        blockMgr->writeBlock(inode.directBlocks[i], buf + bytesWritten, chunk);
        bytesWritten += chunk;
    }

    inode.size         = newSize;
    inode.modifiedTime = time(nullptr);
    return inodeMgr->writeInode(dirInode, inode);
}

// ── Public API ────────────────────────────────────────────────────────────────

int DirectoryHandler::createDirectory() {
    int inodeNumber = inodeMgr->allocateInode(TYPE_DIRECTORY);
    if (inodeNumber == -1) {
        cerr << "[DirectoryHandler] ERROR: Failed to allocate directory inode" << endl;
        return -1;
    }

    Inode inode;
    if (!inodeMgr->readInode((unsigned int)inodeNumber, inode)) {
        inodeMgr->deallocateInode((unsigned int)inodeNumber);
        return -1;
    }
    inode.size       = 0;
    inode.blockCount = 0;
    inodeMgr->writeInode((unsigned int)inodeNumber, inode);

    return inodeNumber;
}

bool DirectoryHandler::deleteDirectory(unsigned int inodeNumber) {
    if (!inodeMgr->inodeExists(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Inode " << inodeNumber << " doesn't exist" << endl;
        return false;
    }
    if (!isEmpty(inodeNumber)) {
        cerr << "[DirectoryHandler] ERROR: Directory not empty" << endl;
        return false;
    }

    Inode inode;
    if (!inodeMgr->readInode(inodeNumber, inode)) return false;

    for (unsigned int i = 0; i < DIRECT_BLOCKS; i++) {
        if (inode.directBlocks[i] != NULL_BLOCK) {
            blockMgr->deallocateBlock(inode.directBlocks[i]);
        }
    }
    return inodeMgr->deallocateInode(inodeNumber);
}

bool DirectoryHandler::addEntry(unsigned int dirInodeNumber, const string& name,
                                unsigned int entryInodeNumber, InodeType type) {
    if (name.length() > MAX_FILENAME_LENGTH) {
        cerr << "[DirectoryHandler] ERROR: Filename too long" << endl;
        return false;
    }
    if (findEntry(dirInodeNumber, name) != -1) {
        cerr << "[DirectoryHandler] ERROR: '" << name << "' already exists" << endl;
        return false;
    }

    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) return false;

    // Build new entry
    DirectoryEntry newEntry;
    newEntry.inodeNumber = entryInodeNumber;
    newEntry.fileType    = (uint8_t)type;
    newEntry.nameLength  = (uint8_t)name.length();
    newEntry.entryLength = sizeof(DirectoryEntry);
    strncpy(newEntry.name, name.c_str(), MAX_FILENAME_LENGTH);
    newEntry.name[name.length()] = '\0';

    unsigned int oldSize = dirInode.size;
    unsigned int newSize = oldSize + sizeof(DirectoryEntry);

    // Build merged buffer
    char* buf = new char[newSize];
    memset(buf, 0, newSize);

    if (oldSize > 0) {
        unsigned int loaded;
        char* existing = loadEntries(dirInodeNumber, loaded);
        if (existing) {
            memcpy(buf, existing, loaded);
            delete[] existing;
        }
    }
    memcpy(buf + oldSize, &newEntry, sizeof(DirectoryEntry));

    bool ok = flushEntries(dirInodeNumber, buf, newSize, dirInode);
    delete[] buf;
    return ok;
}

bool DirectoryHandler::removeEntry(unsigned int dirInodeNumber, const string& name) {
    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);

    bool found = false;
    vector<DirectoryEntry> remaining;
    for (const DirectoryEntry& e : entries) {
        if (string(e.name) == name) { found = true; }
        else                        { remaining.push_back(e); }
    }

    if (!found) {
        cerr << "[DirectoryHandler] ERROR: '" << name << "' not found" << endl;
        return false;
    }

    Inode dirInode;
    if (!inodeMgr->readInode(dirInodeNumber, dirInode)) return false;

    unsigned int newSize = (unsigned int)(remaining.size() * sizeof(DirectoryEntry));
    char* buf = new char[newSize > 0 ? newSize : 1];
    memset(buf, 0, newSize > 0 ? newSize : 1);

    for (size_t i = 0; i < remaining.size(); i++) {
        memcpy(buf + i * sizeof(DirectoryEntry), &remaining[i], sizeof(DirectoryEntry));
    }

    bool ok = flushEntries(dirInodeNumber, buf, newSize, dirInode);
    delete[] buf;
    return ok;
}

int DirectoryHandler::findEntry(unsigned int dirInodeNumber, const string& name) {
    vector<DirectoryEntry> entries = listDirectory(dirInodeNumber);
    for (const DirectoryEntry& e : entries) {
        if (string(e.name) == name) return (int)e.inodeNumber;
    }
    return -1;
}

vector<DirectoryEntry> DirectoryHandler::listDirectory(unsigned int dirInodeNumber) {
    vector<DirectoryEntry> entries;
    if (!inodeMgr->inodeExists(dirInodeNumber)) return entries;

    unsigned int size;
    char* buf = loadEntries(dirInodeNumber, size);
    if (!buf || size == 0) { delete[] buf; return entries; }

    unsigned int count = size / sizeof(DirectoryEntry);
    for (unsigned int i = 0; i < count; i++) {
        DirectoryEntry e;
        memcpy(&e, buf + i * sizeof(DirectoryEntry), sizeof(DirectoryEntry));
        entries.push_back(e);
    }
    delete[] buf;
    return entries;
}

bool DirectoryHandler::isEmpty(unsigned int dirInodeNumber) {
    return getEntryCount(dirInodeNumber) == 0;
}

unsigned int DirectoryHandler::getEntryCount(unsigned int dirInodeNumber) {
    Inode inode;
    if (!inodeMgr->readInode(dirInodeNumber, inode)) return 0;
    return inode.size / sizeof(DirectoryEntry);
}

// ── Search ────────────────────────────────────────────────────────────────────

void DirectoryHandler::searchRecursive(unsigned int dirInode,
                                       const string& target,
                                       const string& currentPath,
                                       bool matchFiles,
                                       bool matchDirs,
                                       vector<string>& results) {
    vector<DirectoryEntry> entries = listDirectory(dirInode);
    for (const DirectoryEntry& e : entries) {
        string eName = string(e.name);
        string full  = (currentPath == "/") ? ("/" + eName) : (currentPath + "/" + eName);

        if (eName == target) {
            bool isDir  = (e.fileType == TYPE_DIRECTORY);
            bool isFile = (e.fileType == TYPE_FILE);
            if ((isFile && matchFiles) || (isDir && matchDirs)) results.push_back(full);
        }

        if (e.fileType == TYPE_DIRECTORY) {
            searchRecursive(e.inodeNumber, target, full, matchFiles, matchDirs, results);
        }
    }
}

vector<string> DirectoryHandler::search(const string& name, bool matchFiles, bool matchDirs) {
    vector<string> results;
    if (!name.empty()) searchRecursive(0, name, "/", matchFiles, matchDirs, results);
    return results;
}

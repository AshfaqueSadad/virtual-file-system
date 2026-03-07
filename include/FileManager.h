#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "InodeManager.h"
#include "BlockManager.h"
#include "EncryptionManager.h"
#include <string>
#include <vector>

class FileManager {
private:
    VirtualDisk* disk;
    InodeManager* inodeMgr;
    BlockManager* blockMgr;
    EncryptionManager* encryptMgr;   // NEW: encryption support (may be nullptr)

public:
    // Constructor - encryptMgr may be nullptr (encryption off)
    FileManager(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                BlockManager* blockMgr, EncryptionManager* encryptMgr = nullptr);

    // Destructor
    ~FileManager();

    // Create a new file (returns inode number or -1 on failure)
    int createFile();

    // Delete a file
    bool deleteFile(unsigned int inodeNumber);

    // Write data to a file (data is encrypted if encryptMgr is active)
    bool writeFile(unsigned int inodeNumber, const char* data, unsigned int size);

    // Read data from a file (data is decrypted if encryptMgr is active)
    bool readFile(unsigned int inodeNumber, char* buffer, unsigned int size);

    // Append data to a file
    bool appendFile(unsigned int inodeNumber, const char* data, unsigned int size);

    // Get file size
    unsigned int getFileSize(unsigned int inodeNumber);

    // Truncate file (set size to 0, deallocate all blocks)
    bool truncateFile(unsigned int inodeNumber);
};

#endif // FILEMANAGER_H

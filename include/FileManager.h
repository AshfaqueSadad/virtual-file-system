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
    VirtualDisk*       disk;
    InodeManager*      inodeMgr;
    BlockManager*      blockMgr;
    EncryptionManager* encryptMgr;   // May be nullptr (encryption disabled)

    // Read raw block data for 'inode' into a caller-supplied buffer
    bool readBlocks(const Inode& inode, char* buf, unsigned int size);

    // Write raw block data from a caller-supplied buffer into 'inode'
    bool writeBlocks(Inode& inode, const char* buf, unsigned int size);

public:
    // encryptMgr may be nullptr — encryption is then skipped
    FileManager(VirtualDisk* virtualDisk, InodeManager* inodeMgr,
                BlockManager* blockMgr, EncryptionManager* encryptMgr = nullptr);
    ~FileManager();

    // Allocate a new empty file inode — returns inode number or -1
    int  createFile();

    // Free all blocks and the inode for a file
    bool deleteFile(unsigned int inodeNumber);

    // Overwrite file content (encrypts if enabled)
    bool writeFile(unsigned int inodeNumber, const char* data, unsigned int size);

    // Read file content into buffer (decrypts if enabled)
    bool readFile(unsigned int inodeNumber, char* buffer, unsigned int size);

    // Append data to existing file content
    bool appendFile(unsigned int inodeNumber, const char* data, unsigned int size);

    // Return the size of the file in bytes
    unsigned int getFileSize(unsigned int inodeNumber);

    // Deallocate all blocks and reset size to 0
    bool truncateFile(unsigned int inodeNumber);
};

#endif // FILEMANAGER_H

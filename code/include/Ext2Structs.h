#ifndef EXT2STRUCTS_H
#define EXT2STRUCTS_H

#include "Constants.h"   // All numeric constants live there
#include <cstdint>
#include <ctime>

// File types
enum InodeType {
    TYPE_FILE      = 1,
    TYPE_DIRECTORY = 2
};

// Superblock structure - stores file system metadata
struct Superblock {
    uint32_t magic;                    // Magic number (0xEF53)
    uint32_t totalBlocks;              // Total number of blocks
    uint32_t totalInodes;              // Total number of inodes
    uint32_t freeBlocks;               // Number of free blocks
    uint32_t freeInodes;               // Number of free inodes
    uint32_t blockSize;                // Size of each block in bytes
    uint32_t inodeSize;                // Size of each inode in bytes

    // Offsets in the disk (in bytes)
    uint32_t inodeBitmapOffset;        // Offset to inode bitmap
    uint32_t blockBitmapOffset;        // Offset to block bitmap
    uint32_t inodeTableOffset;         // Offset to inode table
    uint32_t dataBlocksOffset;         // Offset to data blocks region

    uint32_t rootInodeNumber;          // Root directory inode number (always 0)

    // Padding to make superblock exactly 1 block (1024 bytes)
    char padding[1024 - 11 * sizeof(uint32_t)];

    // Default constructor
    Superblock() {
        magic           = EXT2_MAGIC;
        totalBlocks     = TOTAL_BLOCKS;
        totalInodes     = TOTAL_INODES;
        freeBlocks      = 0;
        freeInodes      = 0;
        blockSize       = BLOCK_SIZE;
        inodeSize       = INODE_SIZE;
        inodeBitmapOffset = 0;
        blockBitmapOffset = 0;
        inodeTableOffset  = 0;
        dataBlocksOffset  = 0;
        rootInodeNumber   = 0;

        for (size_t i = 0; i < sizeof(padding); i++) {
            padding[i] = 0;
        }
    }
};

// Inode structure - stores metadata for files and directories
struct Inode {
    uint16_t type;                         // File type (file or directory)
    uint16_t permissions;                  // File permissions
    uint32_t size;                         // File size in bytes
    uint32_t blockCount;                   // Number of allocated blocks

    time_t createdTime;                    // Creation timestamp
    time_t modifiedTime;                   // Last modification timestamp
    time_t accessedTime;                   // Last access timestamp

    uint32_t directBlocks[DIRECT_BLOCKS];  // Direct block pointers
    uint32_t singleIndirect;               // Single indirect block pointer
    uint32_t doubleIndirect;               // Double indirect block pointer

    // Padding to make inode exactly INODE_SIZE (128) bytes
    char padding[128 - (2 * sizeof(uint16_t) + 2 * sizeof(uint32_t) +
                 3 * sizeof(time_t) + (DIRECT_BLOCKS + 2) * sizeof(uint32_t))];

    // Default constructor — all block pointers set to NULL_BLOCK
    Inode() {
        type          = 0;
        permissions   = 0;
        size          = 0;
        blockCount    = 0;
        createdTime   = 0;
        modifiedTime  = 0;
        accessedTime  = 0;
        singleIndirect = NULL_BLOCK;
        doubleIndirect = NULL_BLOCK;

        for (unsigned int i = 0; i < DIRECT_BLOCKS; i++) {
            directBlocks[i] = NULL_BLOCK;
        }
        for (size_t i = 0; i < sizeof(padding); i++) {
            padding[i] = 0;
        }
    }
};

// Directory entry structure - maps filenames to inode numbers
struct DirectoryEntry {
    uint32_t inodeNumber;               // Inode number of this entry
    uint16_t entryLength;               // Length of this directory entry record
    uint8_t  nameLength;                // Length of the filename
    uint8_t  fileType;                  // File type (TYPE_FILE or TYPE_DIRECTORY)
    char     name[MAX_FILENAME_LENGTH + 1]; // Null-terminated filename

    // Default constructor
    DirectoryEntry() {
        inodeNumber = 0;
        entryLength = 0;
        nameLength  = 0;
        fileType    = 0;
        for (size_t i = 0; i < MAX_FILENAME_LENGTH + 1; i++) {
            name[i] = '\0';
        }
    }
};

// Bitmap utilities — inline bit-manipulation helpers
class BitmapUtils {
public:
    static void setBit(char* bitmap, unsigned int index) {
        bitmap[index / 8] |= (1 << (index % 8));
    }

    static void clearBit(char* bitmap, unsigned int index) {
        bitmap[index / 8] &= ~(1 << (index % 8));
    }

    static bool testBit(const char* bitmap, unsigned int index) {
        return (bitmap[index / 8] & (1 << (index % 8))) != 0;
    }

    // Returns index of first free bit, or -1 if all are used
    static int findFirstFree(const char* bitmap, unsigned int maxBits) {
        for (unsigned int i = 0; i < maxBits; i++) {
            if (!testBit(bitmap, i)) {
                return (int)i;
            }
        }
        return -1;
    }
};

#endif // EXT2STRUCTS_H

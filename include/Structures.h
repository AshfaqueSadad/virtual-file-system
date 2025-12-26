#ifndef STRUCTURES_H
#define STRUCTURES_H

const int BLOCK_SIZE = 1024;        // 1KB blocks
const int MAX_FILES = 16;          
const int DATA_START_BLOCK = 20;    // Reserve first 20 blocks for metadata

struct FileEntry {  //metadata for the filesystem
    char name[32];
    int startBlock;
    int size;
    bool isUsed;
};


struct Superblock {
    int magicNumber; // 0xEF53 magic number for the file system 
    int fileCount;
};

#endif  
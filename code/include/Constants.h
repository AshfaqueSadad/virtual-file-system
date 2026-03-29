#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

// ── Disk geometry ─────────────────────────────────────────────────────────────
const unsigned int BLOCK_SIZE         = 1024;    // Bytes per block
const unsigned int TOTAL_BLOCKS       = 10240;   // 10 MB / 1 KB
const unsigned int TOTAL_INODES       = 1024;    // Max files + directories

// ── Inode layout ──────────────────────────────────────────────────────────────
const unsigned int INODE_SIZE         = 128;     // Bytes per inode
const unsigned int DIRECT_BLOCKS      = 12;      // Direct block pointers per inode
const unsigned int MAX_FILENAME_LENGTH = 255;    // Max chars in a filename

// ── Sentinel / magic values ───────────────────────────────────────────────────
// NULL_BLOCK uses 0xFFFFFFFF so that block 0 can be a valid real data block.
const uint32_t NULL_BLOCK  = 0xFFFFFFFF;
const uint32_t EXT2_MAGIC  = 0xEF53;            // EXT2 filesystem magic number

#endif // CONSTANTS_H

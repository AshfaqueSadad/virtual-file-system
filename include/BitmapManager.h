#ifndef BITMAPMANAGER_H
#define BITMAPMANAGER_H

#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include <iostream>

class BitmapManager {
private:
    VirtualDisk* disk;
    unsigned int bitmapOffset;     // Offset of bitmap on disk
    unsigned int bitmapSize;       // Size of bitmap in bytes
    unsigned int maxItems;         // Maximum number of items this bitmap tracks
    char* bitmapCache;             // In-memory cache of bitmap
    bool isLoaded;

public:
    // Constructor
    BitmapManager(VirtualDisk* virtualDisk, unsigned int offset, unsigned int items);
    
    // Destructor
    ~BitmapManager();
    
    // Initialize bitmap (all bits set to 0 - free)
    bool initialize();
    
    // Load bitmap from disk into memory
    bool load();
    
    // Save bitmap from memory to disk
    bool save();
    
    // Allocate a free item (find first free bit, mark as used, return index)
    int allocate();
    
    // Deallocate an item (mark bit as free)
    bool deallocate(unsigned int index);
    
    // Check if an item is free
    bool isFree(unsigned int index) const;
    
    // Check if an item is used
    bool isUsed(unsigned int index) const;
    
    // Get number of free items
    unsigned int countFree() const;
    
    // Get number of used items
    unsigned int countUsed() const;
    
    // Print bitmap status (for debugging)
    void printStatus() const;
};

#endif // BITMAPMANAGER_H

#include "BitmapManager.h"
#include <cstring>

using namespace std;

// Constructor
BitmapManager::BitmapManager(VirtualDisk* virtualDisk, unsigned int offset, unsigned int items)
    : disk(virtualDisk), bitmapOffset(offset), maxItems(items), isLoaded(false) {
    
    // Calculate bitmap size in bytes (1 bit per item, so divide by 8)
    bitmapSize = (maxItems + 7) / 8;  // Round up
    
    // Allocate memory for bitmap cache
    bitmapCache = new char[bitmapSize];
    memset(bitmapCache, 0, bitmapSize);
    
    cout << "[BitmapManager] Initialized at offset " << offset 
         << ", tracking " << items << " items, bitmap size: " 
         << bitmapSize << " bytes" << endl;
}

// Destructor
BitmapManager::~BitmapManager() {
    if (bitmapCache != nullptr) {
        delete[] bitmapCache;
        bitmapCache = nullptr;
    }
    cout << "[BitmapManager] Destroyed" << endl;
}

// Initialize bitmap (set all to free)
bool BitmapManager::initialize() {
    cout << "[BitmapManager] Initializing bitmap (all free)" << endl;
    
    // Set all bits to 0 (free)
    memset(bitmapCache, 0, bitmapSize);
    
    // Save to disk
    if (!save()) {
        cerr << "[BitmapManager] ERROR: Failed to save initialized bitmap" << endl;
        return false;
    }
    
    isLoaded = true;
    cout << "[BitmapManager] Bitmap initialized successfully" << endl;
    return true;
}

// Load bitmap from disk
bool BitmapManager::load() {
    cout << "[BitmapManager] Loading bitmap from disk at offset " << bitmapOffset << endl;
    
    if (!disk->readBlock(bitmapOffset, bitmapCache, bitmapSize)) {
        cerr << "[BitmapManager] ERROR: Failed to load bitmap from disk" << endl;
        return false;
    }
    
    isLoaded = true;
    cout << "[BitmapManager] Bitmap loaded successfully" << endl;
    return true;
}

// Save bitmap to disk
bool BitmapManager::save() {
    cout << "[BitmapManager] Saving bitmap to disk at offset " << bitmapOffset << endl;
    
    if (!disk->writeBlock(bitmapOffset, bitmapCache, bitmapSize)) {
        cerr << "[BitmapManager] ERROR: Failed to save bitmap to disk" << endl;
        return false;
    }
    
    disk->flush();
    cout << "[BitmapManager] Bitmap saved successfully" << endl;
    return true;
}

// Allocate a free item
int BitmapManager::allocate() {
    if (!isLoaded) {
        cerr << "[BitmapManager] ERROR: Bitmap not loaded" << endl;
        return -1;
    }
    
    // Find first free bit
    int freeIndex = BitmapUtils::findFirstFree(bitmapCache, maxItems);
    
    if (freeIndex == -1) {
        cerr << "[BitmapManager] ERROR: No free items available" << endl;
        return -1;
    }
    
    // Mark as used
    BitmapUtils::setBit(bitmapCache, freeIndex);
    
    cout << "[BitmapManager] Allocated item at index " << freeIndex << endl;
    
    // Save to disk
    save();
    
    return freeIndex;
}

// Deallocate an item
bool BitmapManager::deallocate(unsigned int index) {
    if (!isLoaded) {
        cerr << "[BitmapManager] ERROR: Bitmap not loaded" << endl;
        return false;
    }
    
    if (index >= maxItems) {
        cerr << "[BitmapManager] ERROR: Index " << index << " out of range" << endl;
        return false;
    }
    
    if (!isUsed(index)) {
        cerr << "[BitmapManager] WARNING: Item " << index << " already free" << endl;
        return false;
    }
    
    // Clear the bit
    BitmapUtils::clearBit(bitmapCache, index);
    
    cout << "[BitmapManager] Deallocated item at index " << index << endl;
    
    // Save to disk
    save();
    
    return true;
}

// Check if item is free
bool BitmapManager::isFree(unsigned int index) const {
    if (index >= maxItems) {
        return false;
    }
    
    return !BitmapUtils::testBit(bitmapCache, index);
}

// Check if item is used
bool BitmapManager::isUsed(unsigned int index) const {
    if (index >= maxItems) {
        return false;
    }
    
    return BitmapUtils::testBit(bitmapCache, index);
}

// Count free items
unsigned int BitmapManager::countFree() const {
    unsigned int count = 0;
    
    for (unsigned int i = 0; i < maxItems; i++) {
        if (isFree(i)) {
            count++;
        }
    }
    
    return count;
}

// Count used items
unsigned int BitmapManager::countUsed() const {
    unsigned int count = 0;
    
    for (unsigned int i = 0; i < maxItems; i++) {
        if (isUsed(i)) {
            count++;
        }
    }
    
    return count;
}

// Print bitmap status
void BitmapManager::printStatus() const {
    cout << "[BitmapManager] Status: "
         << countUsed() << " used, "
         << countFree() << " free (total: " << maxItems << ")" << endl;
}

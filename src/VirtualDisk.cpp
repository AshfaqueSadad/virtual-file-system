#include "VirtualDisk.h"
#include <iostream>
#include <cstring>

using namespace std;

// Constructor
VirtualDisk::VirtualDisk(const string& fileName, unsigned int size) 
    : diskFileName(fileName), diskSize(size), isOpen(false) {
    cout << "[VirtualDisk] Initialized with file: " << fileName 
         << ", size: " << size << " bytes" << endl;
}

// Destructor
VirtualDisk::~VirtualDisk() {
    if (isOpen) {
        close();
    }
    cout << "[VirtualDisk] Destroyed" << endl;
}

// Create a new disk file
bool VirtualDisk::create() {
    cout << "[VirtualDisk] Creating new disk file: " << diskFileName << endl;
    
    // Open file in write mode
    diskFile.open(diskFileName, ios::out | ios::binary);
    
    if (!diskFile.is_open()) {
        cerr << "[VirtualDisk] ERROR: Failed to create disk file" << endl;
        return false;
    }
    
    // Write zeros to initialize the disk
    char zeroBlock[4096];
    memset(zeroBlock, 0, 4096);
    
    unsigned int blocksToWrite = diskSize / 4096;
    unsigned int remainder = diskSize % 4096;
    
    cout << "[VirtualDisk] Writing " << blocksToWrite << " blocks of 4096 bytes" << endl;
    
    for (unsigned int i = 0; i < blocksToWrite; i++) {
        diskFile.write(zeroBlock, 4096);
        
        if (diskFile.fail()) {
            cerr << "[VirtualDisk] ERROR: Failed to write block " << i << endl;
            diskFile.close();
            return false;
        }
    }
    
    // Write remaining bytes
    if (remainder > 0) {
        diskFile.write(zeroBlock, remainder);
        cout << "[VirtualDisk] Wrote " << remainder << " remaining bytes" << endl;
    }
    
    diskFile.close();
    
    cout << "[VirtualDisk] Disk file created successfully" << endl;
    
    // Now open it for read/write
    return open();
}

// Open existing disk file
bool VirtualDisk::open() {
    cout << "[VirtualDisk] Opening disk file: " << diskFileName << endl;
    
    diskFile.open(diskFileName, ios::in | ios::out | ios::binary);
    
    if (!diskFile.is_open()) {
        cerr << "[VirtualDisk] ERROR: Failed to open disk file" << endl;
        return false;
    }
    
    // Verify file size
    diskFile.seekg(0, ios::end);
    unsigned int fileSize = diskFile.tellg();
    diskFile.seekg(0, ios::beg);
    
    if (fileSize != diskSize) {
        cerr << "[VirtualDisk] WARNING: Disk file size mismatch. Expected: " 
             << diskSize << ", Got: " << fileSize << endl;
    }
    
    isOpen = true;
    cout << "[VirtualDisk] Disk file opened successfully" << endl;
    return true;
}

// Close disk file
void VirtualDisk::close() {
    if (isOpen) {
        cout << "[VirtualDisk] Closing disk file" << endl;
        diskFile.close();
        isOpen = false;
    }
}

// Read block from disk
bool VirtualDisk::readBlock(unsigned int offset, char* buffer, unsigned int size) {
    if (!isOpen) {
        cerr << "[VirtualDisk] ERROR: Disk not open" << endl;
        return false;
    }
    
    if (offset + size > diskSize) {
        cerr << "[VirtualDisk] ERROR: Read beyond disk boundary. Offset: " 
             << offset << ", Size: " << size << ", Disk size: " << diskSize << endl;
        return false;
    }
    
    cout << "[VirtualDisk] Reading " << size << " bytes from offset " << offset << endl;
    
    diskFile.seekg(offset, ios::beg);
    diskFile.read(buffer, size);
    
    if (diskFile.fail()) {
        cerr << "[VirtualDisk] ERROR: Failed to read from disk" << endl;
        return false;
    }
    
    return true;
}

// Write block to disk
bool VirtualDisk::writeBlock(unsigned int offset, const char* buffer, unsigned int size) {
    if (!isOpen) {
        cerr << "[VirtualDisk] ERROR: Disk not open" << endl;
        return false;
    }
    
    if (offset + size > diskSize) {
        cerr << "[VirtualDisk] ERROR: Write beyond disk boundary. Offset: " 
             << offset << ", Size: " << size << ", Disk size: " << diskSize << endl;
        return false;
    }
    
    cout << "[VirtualDisk] Writing " << size << " bytes to offset " << offset << endl;
    
    diskFile.seekp(offset, ios::beg);
    diskFile.write(buffer, size);
    
    if (diskFile.fail()) {
        cerr << "[VirtualDisk] ERROR: Failed to write to disk" << endl;
        return false;
    }
    
    return true;
}

// Get disk size
unsigned int VirtualDisk::getSize() const {
    return diskSize;
}

// Check disk status
bool VirtualDisk::getDiskStatus() const {
    return isOpen;
}

// Flush disk changes
void VirtualDisk::flush() {
    if (isOpen) {
        cout << "[VirtualDisk] Flushing disk changes" << endl;
        diskFile.flush();
    }
}

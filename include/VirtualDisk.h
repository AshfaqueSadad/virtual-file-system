#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H

#include <string>
#include <fstream>

class VirtualDisk {
private:
    std::string diskFileName;
    std::fstream diskFile;
    unsigned int diskSize;
    bool isOpen;

public:
    // Constructor
    VirtualDisk(const std::string& fileName, unsigned int size);
    
    // Destructor
    ~VirtualDisk();
    
    // Initialize and create a new disk file
    bool create();
    
    // Open existing disk file
    bool open();
    
    // Close disk file
    void close();
    
    // Read data from disk at specific offset
    bool readBlock(unsigned int offset, char* buffer, unsigned int size);
    
    // Write data to disk at specific offset
    bool writeBlock(unsigned int offset, const char* buffer, unsigned int size);
    
    // Get disk size
    unsigned int getSize() const;
    
    // Check if disk is open
    bool getDiskStatus() const;
    
    // Flush changes to disk
    void flush();
};

#endif // VIRTUALDISK_H

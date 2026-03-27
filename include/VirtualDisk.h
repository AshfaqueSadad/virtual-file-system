#ifndef VIRTUALDISK_H
#define VIRTUALDISK_H

#include <string>
#include <fstream>

class VirtualDisk {
private:
    std::string  diskFileName;
    std::fstream diskFile;
    unsigned int diskSize;
    bool         isOpen;

public:
    VirtualDisk(const std::string& fileName, unsigned int size);
    ~VirtualDisk();

    // Create a new blank disk image
    bool create();

    // Open an existing disk image
    bool open();

    // Close the disk image
    void close();

    // Low-level read at byte offset
    bool readBlock(unsigned int offset, char* buffer, unsigned int size);

    // Low-level write at byte offset
    bool writeBlock(unsigned int offset, const char* buffer, unsigned int size);

    // Flush pending writes to the underlying file
    void flush();

    // Return the total disk capacity in bytes
    unsigned int getSize() const;

    // Return true if the disk image is currently open
    bool getDiskStatus() const;

    // Alias for getDiskStatus() — more expressive name
    bool isReady() const;

    // Return the disk image filename
    std::string getDiskFileName() const;
};

#endif // VIRTUALDISK_H

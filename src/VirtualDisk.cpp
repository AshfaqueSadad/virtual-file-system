#include "VirtualDisk.h"
#include <iostream>
#include <cstring>

using namespace std;

VirtualDisk::VirtualDisk(const string& fileName, unsigned int size)
    : diskFileName(fileName), diskSize(size), isOpen(false) {}

VirtualDisk::~VirtualDisk() {
    if (isOpen) close();
}

bool VirtualDisk::create() {
    diskFile.open(diskFileName, ios::out | ios::binary);
    if (!diskFile.is_open()) {
        cerr << "[VirtualDisk] ERROR: Failed to create disk file" << endl;
        return false;
    }

    char zeroBlock[4096];
    memset(zeroBlock, 0, sizeof(zeroBlock));

    unsigned int blocksToWrite = diskSize / 4096;
    unsigned int remainder     = diskSize % 4096;

    for (unsigned int i = 0; i < blocksToWrite; i++) {
        diskFile.write(zeroBlock, 4096);
        if (diskFile.fail()) {
            cerr << "[VirtualDisk] ERROR: Failed to write zero block " << i << endl;
            diskFile.close();
            return false;
        }
    }
    if (remainder > 0) diskFile.write(zeroBlock, remainder);

    diskFile.close();
    return open();
}

bool VirtualDisk::open() {
    diskFile.open(diskFileName, ios::in | ios::out | ios::binary);
    if (!diskFile.is_open()) {
        cerr << "[VirtualDisk] ERROR: Failed to open disk file" << endl;
        return false;
    }

    diskFile.seekg(0, ios::end);
    unsigned int fileSize = (unsigned int)diskFile.tellg();
    diskFile.seekg(0, ios::beg);

    if (fileSize != diskSize) {
        cerr << "[VirtualDisk] WARNING: Size mismatch. Expected "
             << diskSize << ", got " << fileSize << endl;
    }

    isOpen = true;
    return true;
}

void VirtualDisk::close() {
    if (isOpen) {
        diskFile.close();
        isOpen = false;
    }
}

bool VirtualDisk::readBlock(unsigned int offset, char* buffer, unsigned int size) {
    if (!isOpen) {
        cerr << "[VirtualDisk] ERROR: Disk not open" << endl;
        return false;
    }
    if (offset + size > diskSize) {
        cerr << "[VirtualDisk] ERROR: Read beyond disk boundary (offset="
             << offset << " size=" << size << ")" << endl;
        return false;
    }
    diskFile.seekg(offset, ios::beg);
    diskFile.read(buffer, size);
    if (diskFile.fail()) {
        cerr << "[VirtualDisk] ERROR: Read failed at offset " << offset << endl;
        return false;
    }
    return true;
}

bool VirtualDisk::writeBlock(unsigned int offset, const char* buffer, unsigned int size) {
    if (!isOpen) {
        cerr << "[VirtualDisk] ERROR: Disk not open" << endl;
        return false;
    }
    if (offset + size > diskSize) {
        cerr << "[VirtualDisk] ERROR: Write beyond disk boundary (offset="
             << offset << " size=" << size << ")" << endl;
        return false;
    }
    diskFile.seekp(offset, ios::beg);
    diskFile.write(buffer, size);
    if (diskFile.fail()) {
        cerr << "[VirtualDisk] ERROR: Write failed at offset " << offset << endl;
        return false;
    }
    return true;
}

void VirtualDisk::flush() {
    if (isOpen) diskFile.flush();
}

unsigned int VirtualDisk::getSize() const { return diskSize; }

bool VirtualDisk::getDiskStatus() const { return isOpen; }

bool VirtualDisk::isReady() const { return isOpen; }

string VirtualDisk::getDiskFileName() const { return diskFileName; }

#include "DiskManager.h"

DiskManager::DiskManager(std::string path) {
    disk.open(path, std::ios::in | std::ios::out | std::ios::binary);
}

void DiskManager::writeRaw(int offset, const char* data, int size) {
    disk.seekp(offset);
    disk.write(data, size);
    disk.flush(); 
}

void DiskManager::readRaw(int offset, char* buffer, int size) {
    disk.seekg(offset);
    disk.read(buffer, size);
}

DiskManager::~DiskManager() { if(disk.is_open()) disk.close(); }
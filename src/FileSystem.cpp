#include "FileSystem.h"
#include <cstring>
#include <iostream>

bool FileSystem::createFile(std::string name, std::string content) {
    FileEntry entry;
    int entryOffset = -1;

    
    for (int i = 0; i < MAX_FILES; i++) {
        int offset = sizeof(Superblock) + (i * sizeof(FileEntry));
        dm->readRaw(offset, reinterpret_cast<char*>(&entry), sizeof(FileEntry));
        
        if (!entry.isUsed) {
            entryOffset = offset;
            entry.startBlock = DATA_START_BLOCK + i; 
            break;
        }
    }

    if (entryOffset == -1) return false; // Disk full (max files reached)

        strncpy(entry.name, name.c_str(), 31);
    entry.size = content.length();
    entry.isUsed = true;

    
    dm->writeRaw(entryOffset, reinterpret_cast<char*>(&entry), sizeof(FileEntry));
    dm->writeRaw(entry.startBlock * BLOCK_SIZE, content.c_str(), entry.size);

    return true;
}

std::string FileSystem::readFile(std::string name) {
    FileEntry entry;
    for (int i = 0; i < MAX_FILES; i++) {
        int offset = sizeof(Superblock) + (i * sizeof(FileEntry));
        dm->readRaw(offset, reinterpret_cast<char*>(&entry), sizeof(FileEntry));

        if (entry.isUsed && std::string(entry.name) == name) {
            char* buffer = new char[entry.size + 1];
            dm->readRaw(entry.startBlock * BLOCK_SIZE, buffer, entry.size);
            buffer[entry.size] = '\0';
            std::string result(buffer);
            delete[] buffer;
            return result;
        }
    }
    return "Error: File not found.";
}
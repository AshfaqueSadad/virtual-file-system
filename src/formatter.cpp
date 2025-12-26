#include <iostream>
#include <fstream>
#include <vector>
#include "Structures.h"

using namespace std;

void formatDisk(const string& diskName) {
    ofstream disk(diskName, ios::binary);
    if (!disk) {
        cerr << "Error: Could not create disk file." << endl;
        return;
    }

   
    vector<char> emptyDisk(10 * 1024 * 1024, 0); 
    disk.write(emptyDisk.data(), emptyDisk.size());

    // 2. Initialize the Superblock
    Superblock sb;
    sb.magicNumber = 0xEF53;
    sb.fileCount = 0;
    
    // Write it at the very beginning using the pointer
    disk.seekp(0);
    disk.write(reinterpret_cast<char*>(&sb), sizeof(Superblock));

    
    FileEntry emptyEntry;
    emptyEntry.isUsed = false;
    for(int i = 0; i < MAX_FILES; i++) {
        disk.write(reinterpret_cast<char*>(&emptyEntry), sizeof(FileEntry));
    }

    disk.close();
    cout << "Disk formatted successfully!" << endl;
}

int main() {
    formatDisk("disk.img");
    return 0;
}
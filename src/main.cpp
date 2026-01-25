#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include "VirtualDisk.h"
#include "Ext2Structs.h"
#include "BitmapManager.h"
#include "SuperblockManager.h"
#include "InodeManager.h"
#include "BlockManager.h"
#include "FileManager.h"
#include "DirectoryHandler.h"
#include "PathParser.h"

using namespace std;

// Global managers
VirtualDisk* disk = nullptr;
SuperblockManager* superblockMgr = nullptr;
BitmapManager* inodeBitmap = nullptr;
BitmapManager* blockBitmap = nullptr;
InodeManager* inodeMgr = nullptr;
BlockManager* blockMgr = nullptr;
FileManager* fileMgr = nullptr;
DirectoryHandler* dirHandler = nullptr;

// Current directory inode
unsigned int currentDirInode = 0;
string currentPath = "/";

// Disable verbose logging
bool verboseMode = false;

// Format the file system
bool formatFileSystem() {
    if (verboseMode) cout << "\n=== FORMATTING FILE SYSTEM ===" << endl;
    
    if (!superblockMgr->initialize()) {
        cerr << "ERROR: Failed to initialize superblock" << endl;
        return false;
    }
    
    if (!inodeBitmap->initialize()) {
        cerr << "ERROR: Failed to initialize inode bitmap" << endl;
        return false;
    }
    
    if (!blockBitmap->initialize()) {
        cerr << "ERROR: Failed to initialize block bitmap" << endl;
        return false;
    }
    
    int rootInode = dirHandler->createDirectory();
    if (rootInode == -1) {
        cerr << "ERROR: Failed to create root directory" << endl;
        return false;
    }
    
    currentDirInode = 0;
    currentPath = "/";
    
    cout << "File system formatted successfully!" << endl;
    return true;
}

// Load existing file system
bool loadFileSystem() {
    if (verboseMode) cout << "\n=== LOADING FILE SYSTEM ===" << endl;
    
    if (!superblockMgr->load()) {
        return false;
    }
    
    if (!inodeBitmap->load()) {
        return false;
    }
    
    if (!blockBitmap->load()) {
        return false;
    }
    
    currentDirInode = 0;
    currentPath = "/";
    
    return true;
}

// Show help menu
void showHelp() {
    cout << "\n=== EXT-2 FILE SYSTEM COMMANDS ===" << endl;
    cout << "format                    - Format the file system (WARNING: erases all data)" << endl;
    cout << "mkdir <path>              - Create a new directory" << endl;
    cout << "touch <file>              - Create a new empty file" << endl;
    cout << "ls [path]                 - List directory contents" << endl;
    cout << "cd <path>                 - Change current directory" << endl;
    cout << "pwd                       - Print working directory" << endl;
    cout << "write <file> <text>       - Write text to a file" << endl;
    cout << "read <file>               - Read and display file contents" << endl;
    cout << "rm <path>                 - Remove a file or empty directory" << endl;
    cout << "info                      - Show file system statistics" << endl;
    cout << "help                      - Show this help menu" << endl;
    cout << "exit                      - Exit the file system simulator" << endl;
    cout << "==================================\n" << endl;
}

// Parse command and arguments
vector<string> parseCommand(const string& input) {
    vector<string> tokens;
    stringstream ss(input);
    string token;
    
    while (ss >> token) {
        tokens.push_back(token);
    }
    
    return tokens;
}

// Handle mkdir command
void cmdMkdir(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: mkdir <path>" << endl;
        return;
    }
    
    string path = args[1];
    string dirName = PathParser::getBasename(path);
    
    // Create directory
    int dirInode = dirHandler->createDirectory();
    if (dirInode == -1) {
        cout << "ERROR: Failed to create directory" << endl;
        return;
    }
    
    // Add to current directory
    if (!dirHandler->addEntry(currentDirInode, dirName, dirInode, TYPE_DIRECTORY)) {
        cout << "ERROR: Failed to add directory entry" << endl;
        inodeMgr->deallocateInode(dirInode);
        return;
    }
    
    cout << "Directory '" << dirName << "' created successfully" << endl;
}

// Handle touch command
void cmdTouch(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: touch <file>" << endl;
        return;
    }
    
    string fileName = args[1];
    
    // Create file
    int fileInode = fileMgr->createFile();
    if (fileInode == -1) {
        cout << "ERROR: Failed to create file" << endl;
        return;
    }
    
    // Add to current directory
    if (!dirHandler->addEntry(currentDirInode, fileName, fileInode, TYPE_FILE)) {
        cout << "ERROR: Failed to add file entry" << endl;
        fileMgr->deleteFile(fileInode);
        return;
    }
    
    cout << "File '" << fileName << "' created successfully" << endl;
}

// Handle ls command
void cmdLs(const vector<string>& args) {
    unsigned int targetInode = currentDirInode;
    
    // Get directory entries
    vector<DirectoryEntry> entries = dirHandler->listDirectory(targetInode);
    
    if (entries.empty()) {
        cout << "(empty directory)" << endl;
        return;
    }
    
    cout << "\nDirectory listing:" << endl;
    cout << "==================" << endl;
    for (const DirectoryEntry& entry : entries) {
        string type = (entry.fileType == TYPE_DIRECTORY) ? "[DIR] " : "[FILE]";
        cout << type << " " << entry.name << endl;
    }
    cout << "\nTotal: " << entries.size() << " entries" << endl;
}

// Handle cd command
void cmdCd(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: cd <path>" << endl;
        return;
    }
    
    string path = args[1];
    
    if (path == "/") {
        currentDirInode = 0;
        currentPath = "/";
        cout << "Changed to root directory" << endl;
        return;
    }
    
    if (path == "..") {
        if (currentDirInode == 0) {
            cout << "Already at root directory" << endl;
            return;
        }
        currentDirInode = 0;
        currentPath = "/";
        cout << "Changed to parent directory" << endl;
        return;
    }
    
    // Find directory in current directory
    int targetInode = dirHandler->findEntry(currentDirInode, path);
    
    if (targetInode == -1) {
        cout << "ERROR: Directory '" << path << "' not found" << endl;
        return;
    }
    
    // Check if it's a directory
    Inode inode;
    if (!inodeMgr->readInode(targetInode, inode)) {
        cout << "ERROR: Failed to read inode" << endl;
        return;
    }
    
    if (inode.type != TYPE_DIRECTORY) {
        cout << "ERROR: '" << path << "' is not a directory" << endl;
        return;
    }
    
    currentDirInode = targetInode;
    if (currentPath == "/") {
        currentPath = "/" + path;
    } else {
        currentPath = currentPath + "/" + path;
    }
    
    cout << "Changed directory to: " << currentPath << endl;
}

// Handle pwd command
void cmdPwd() {
    cout << currentPath << endl;
}

// Handle write command
void cmdWrite(const vector<string>& args) {
    if (args.size() < 3) {
        cout << "Usage: write <file> <text>" << endl;
        return;
    }
    
    string fileName = args[1];
    
    // Reconstruct the text from remaining arguments
    string text = "";
    for (size_t i = 2; i < args.size(); i++) {
        if (i > 2) text += " ";
        text += args[i];
    }
    
    // Find file in current directory
    int fileInode = dirHandler->findEntry(currentDirInode, fileName);
    
    if (fileInode == -1) {
        cout << "ERROR: File '" << fileName << "' not found" << endl;
        return;
    }
    
    // Write to file
    if (!fileMgr->writeFile(fileInode, text.c_str(), text.length())) {
        cout << "ERROR: Failed to write to file" << endl;
        return;
    }
    
    cout << "Wrote " << text.length() << " bytes to '" << fileName << "'" << endl;
}

// Handle read command
void cmdRead(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: read <file>" << endl;
        return;
    }
    
    string fileName = args[1];
    
    // Find file in current directory
    int fileInode = dirHandler->findEntry(currentDirInode, fileName);
    
    if (fileInode == -1) {
        cout << "ERROR: File '" << fileName << "' not found" << endl;
        return;
    }
    
    // Get file size
    unsigned int fileSize = fileMgr->getFileSize(fileInode);
    
    if (fileSize == 0) {
        cout << "(empty file)" << endl;
        return;
    }
    
    // Read file
    char* buffer = new char[fileSize + 1];
    memset(buffer, 0, fileSize + 1);
    
    if (!fileMgr->readFile(fileInode, buffer, fileSize)) {
        cout << "ERROR: Failed to read file" << endl;
        delete[] buffer;
        return;
    }
    
    cout << "\n--- Contents of '" << fileName << "' ---" << endl;
    cout << buffer << endl;
    cout << "--- End of file (" << fileSize << " bytes) ---\n" << endl;
    
    delete[] buffer;
}

// Handle rm command
void cmdRm(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: rm <path>" << endl;
        return;
    }
    
    string name = args[1];
    
    // Find entry in current directory
    int targetInode = dirHandler->findEntry(currentDirInode, name);
    
    if (targetInode == -1) {
        cout << "ERROR: '" << name << "' not found" << endl;
        return;
    }
    
    // Check if it's a file or directory
    Inode inode;
    if (!inodeMgr->readInode(targetInode, inode)) {
        cout << "ERROR: Failed to read inode" << endl;
        return;
    }
    
    if (inode.type == TYPE_DIRECTORY) {
        if (!dirHandler->isEmpty(targetInode)) {
            cout << "ERROR: Directory not empty. Remove contents first." << endl;
            return;
        }
        
        if (!dirHandler->deleteDirectory(targetInode)) {
            cout << "ERROR: Failed to delete directory" << endl;
            return;
        }
    } else {
        if (!fileMgr->deleteFile(targetInode)) {
            cout << "ERROR: Failed to delete file" << endl;
            return;
        }
    }
    
    // Remove entry from current directory
    if (!dirHandler->removeEntry(currentDirInode, name)) {
        cout << "ERROR: Failed to remove directory entry" << endl;
        return;
    }
    
    cout << "'" << name << "' removed successfully" << endl;
}

// Handle info command
void cmdInfo() {
    cout << "\n=== FILE SYSTEM STATISTICS ===" << endl;
    cout << "Inodes: " << inodeBitmap->countUsed() << " used, " 
         << inodeBitmap->countFree() << " free (total: " << TOTAL_INODES << ")" << endl;
    cout << "Blocks: " << blockBitmap->countUsed() << " used, " 
         << blockBitmap->countFree() << " free (total: " << TOTAL_BLOCKS << ")" << endl;
    cout << "Block Size: " << BLOCK_SIZE << " bytes" << endl;
    cout << "Total Disk Size: " << (TOTAL_BLOCKS * BLOCK_SIZE) / 1024 / 1024 << " MB" << endl;
    cout << "==============================\n" << endl;
}

// Main interactive loop
void interactiveShell() {
    string input;
    
    cout << "\nType 'help' for available commands, 'exit' to quit.\n" << endl;
    
    while (true) {
        cout << "ext2sim:" << currentPath << "$ ";
        getline(cin, input);
        
        if (input.empty()) continue;
        
        vector<string> args = parseCommand(input);
        if (args.empty()) continue;
        
        string cmd = args[0];
        
        if (cmd == "exit" || cmd == "quit") {
            cout << "Exiting EXT-2 File System Simulator..." << endl;
            break;
        }
        else if (cmd == "help") {
            showHelp();
        }
        else if (cmd == "format") {
            cout << "WARNING: This will erase all data. Continue? (yes/no): ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "yes") {
                formatFileSystem();
            } else {
                cout << "Format cancelled." << endl;
            }
        }
        else if (cmd == "mkdir") {
            cmdMkdir(args);
        }
        else if (cmd == "touch") {
            cmdTouch(args);
        }
        else if (cmd == "ls") {
            cmdLs(args);
        }
        else if (cmd == "cd") {
            cmdCd(args);
        }
        else if (cmd == "pwd") {
            cmdPwd();
        }
        else if (cmd == "write") {
            cmdWrite(args);
        }
        else if (cmd == "read" || cmd == "cat") {
            cmdRead(args);
        }
        else if (cmd == "rm") {
            cmdRm(args);
        }
        else if (cmd == "info") {
            cmdInfo();
        }
        else {
            cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands." << endl;
        }
    }
}

int main() {
    cout << "===========================================" << endl;
    cout << "  EXT-2 FILE SYSTEM SIMULATOR" << endl;
    cout << "===========================================" << endl;
    
    // Create disk
    const string diskFileName = "disk.img";
    const unsigned int diskSize = 10 * 1024 * 1024;  // 10MB
    
    disk = new VirtualDisk(diskFileName, diskSize);
    
    // Try to open existing disk
    bool diskExists = disk->open();
    
    if (!diskExists) {
        cout << "\nNo existing disk found. Creating new disk..." << endl;
        if (!disk->create()) {
            cerr << "FATAL ERROR: Failed to create disk!" << endl;
            delete disk;
            return 1;
        }
    }
    
    // Create managers
    superblockMgr = new SuperblockManager(disk);
    
    // Check if file system exists
    bool fsExists = false;
    if (diskExists) {
        fsExists = superblockMgr->load() && superblockMgr->validate();
    }
    
    // Create bitmaps
    if (fsExists) {
        inodeBitmap = new BitmapManager(disk, superblockMgr->getInodeBitmapOffset(), TOTAL_INODES);
        blockBitmap = new BitmapManager(disk, superblockMgr->getBlockBitmapOffset(), TOTAL_BLOCKS);
        
        inodeBitmap->load();
        blockBitmap->load();
    } else {
        inodeBitmap = new BitmapManager(disk, BLOCK_SIZE, TOTAL_INODES);
        blockBitmap = new BitmapManager(disk, BLOCK_SIZE * 2, TOTAL_BLOCKS);
    }
    
    // Create other managers
    inodeMgr = new InodeManager(disk, superblockMgr, inodeBitmap);
    blockMgr = new BlockManager(disk, superblockMgr, blockBitmap);
    fileMgr = new FileManager(disk, inodeMgr, blockMgr);
    dirHandler = new DirectoryHandler(disk, inodeMgr, blockMgr, fileMgr);
    
    // Format or load file system
    if (!fsExists) {
        cout << "\nNo valid file system found." << endl;
        cout << "Formatting new file system..." << endl;
        if (!formatFileSystem()) {
            cerr << "FATAL ERROR: Failed to format file system!" << endl;
            goto cleanup;
        }
    } else {
        cout << "\nExisting file system loaded successfully!" << endl;
        if (!loadFileSystem()) {
            cerr << "FATAL ERROR: Failed to load file system!" << endl;
            goto cleanup;
        }
    }
    
    // Start interactive shell
    interactiveShell();
    
cleanup:
    // Cleanup
    delete dirHandler;
    delete fileMgr;
    delete blockMgr;
    delete inodeMgr;
    delete blockBitmap;
    delete inodeBitmap;
    delete superblockMgr;
    delete disk;
    
    return 0;
}

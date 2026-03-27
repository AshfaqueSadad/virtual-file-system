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
#include "EncryptionManager.h"
#include "Logger.h"
#include "ShellHelper.h"

using namespace std;

// ── Global managers ───────────────────────────────────────────────────────────
VirtualDisk*       disk          = nullptr;
SuperblockManager* superblockMgr = nullptr;
BitmapManager*     inodeBitmap   = nullptr;
BitmapManager*     blockBitmap   = nullptr;
InodeManager*      inodeMgr      = nullptr;
BlockManager*      blockMgr      = nullptr;
FileManager*       fileMgr       = nullptr;
DirectoryHandler*  dirHandler    = nullptr;
EncryptionManager* encryptMgr    = nullptr;   // NEW
Logger*            logger        = nullptr;   // NEW

// ── Shell state ───────────────────────────────────────────────────────────────
unsigned int      currentDirInode = 0;
string            currentPath     = "/";
vector<unsigned int> directoryStack;   // stack of parent inode numbers for cd ..

bool verboseMode = false;

// ── Helper: build a log detail string ────────────────────────────────────────
// Identity helper — lets cmd* functions pass string literals to logger cleanly.
static string ld(const string& s) { return s; }

// ── File-system lifecycle ─────────────────────────────────────────────────────

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
    currentPath     = "/";
    directoryStack.clear();

    if (logger) logger->info("FORMAT", "File system formatted. Root inode=0");

    cout << "File system formatted successfully!" << endl;
    return true;
}

bool loadFileSystem() {
    if (verboseMode) cout << "\n=== LOADING FILE SYSTEM ===" << endl;

    if (!superblockMgr->load())   return false;
    if (!inodeBitmap->load())     return false;
    if (!blockBitmap->load())     return false;

    currentDirInode = 0;
    currentPath     = "/";
    directoryStack.clear();

    if (logger) logger->info("LOAD", "Existing file system loaded successfully");
    return true;
}

// ── Help ──────────────────────────────────────────────────────────────────────

void showHelp() {
    ShellHelper::printHelp();
}

// ── Command parser ────────────────────────────────────────────────────────────

vector<string> parseCommand(const string& input) {
    vector<string> tokens;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// ── mkdir ─────────────────────────────────────────────────────────────────────

void cmdMkdir(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: mkdir <name>" << endl;
        return;
    }

    string dirName = args[1];

    if (!PathParser::isValidName(dirName)) {
        cout << "ERROR: Invalid directory name '" << dirName << "'" << endl;
        return;
    }

    int dirInode = dirHandler->createDirectory();
    if (dirInode == -1) {
        cout << "ERROR: Failed to create directory" << endl;
        if (logger) logger->error("MKDIR", "name='" + dirName + "' FAIL: could not allocate inode");
        return;
    }

    if (!dirHandler->addEntry(currentDirInode, dirName, dirInode, TYPE_DIRECTORY)) {
        cout << "ERROR: Failed to add directory entry" << endl;
        inodeMgr->deallocateInode(dirInode);
        if (logger) logger->error("MKDIR", "name='" + dirName + "' FAIL: could not add entry to parent dir");
        return;
    }

    cout << "Directory '" << dirName << "' created successfully" << endl;
    if (logger) logger->info("MKDIR", "name='" + dirName + "' inode=" +
                              to_string(dirInode) + " parent_inode=" +
                              to_string(currentDirInode) + " path=" + currentPath + " SUCCESS");
}

// ── touch ─────────────────────────────────────────────────────────────────────

void cmdTouch(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: touch <file>" << endl;
        return;
    }

    string fileName = args[1];

    if (!PathParser::isValidName(fileName)) {
        cout << "ERROR: Invalid file name '" << fileName << "'" << endl;
        return;
    }

    int fileInode = fileMgr->createFile();
    if (fileInode == -1) {
        cout << "ERROR: Failed to create file" << endl;
        if (logger) logger->error("TOUCH", "name='" + fileName + "' FAIL: could not allocate inode");
        return;
    }

    if (!dirHandler->addEntry(currentDirInode, fileName, fileInode, TYPE_FILE)) {
        cout << "ERROR: Failed to add file entry" << endl;
        fileMgr->deleteFile(fileInode);
        if (logger) logger->error("TOUCH", "name='" + fileName + "' FAIL: could not add entry to parent dir");
        return;
    }

    cout << "File '" << fileName << "' created successfully" << endl;
    if (logger) logger->info("TOUCH", "name='" + fileName + "' inode=" +
                              to_string(fileInode) + " parent_inode=" +
                              to_string(currentDirInode) + " path=" + currentPath + " SUCCESS");
}

// ── ls ────────────────────────────────────────────────────────────────────────

void cmdLs(const vector<string>& args) {
    (void)args;
    unsigned int targetInode = currentDirInode;

    vector<DirectoryEntry> entries = dirHandler->listDirectory(targetInode);

    if (entries.empty()) {
        cout << "(empty directory)" << endl;
        if (logger) logger->info("LS", "path=" + currentPath + " entries=0");
        return;
    }

    cout << "\nDirectory listing:" << endl;
    cout << "==================" << endl;
    for (const DirectoryEntry& entry : entries) {
        ShellHelper::printDirEntry(entry);
    }
    cout << "\nTotal: " << entries.size() << " entries" << endl;

    if (logger) logger->info("LS", "path=" + currentPath +
                              " entries=" + to_string(entries.size()));
}

// ── cd ────────────────────────────────────────────────────────────────────────

void cmdCd(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: cd <path>" << endl;
        return;
    }

    string path = args[1];

    // Navigate to root
    if (path == "/") {
        currentDirInode = 0;
        currentPath     = "/";
        directoryStack.clear();
        cout << "Changed to root directory" << endl;
        if (logger) logger->info("CD", "target='/' SUCCESS");
        return;
    }

    // Navigate to parent
    if (path == "..") {
        if (currentDirInode == 0) {
            cout << "Already at root directory" << endl;
            return;
        }

        if (!directoryStack.empty()) {
            currentDirInode = directoryStack.back();
            directoryStack.pop_back();

            // Rebuild currentPath by stripping last component
            size_t lastSlash = currentPath.find_last_of('/');
            if (lastSlash == 0) {
                currentPath = "/";
            } else if (lastSlash != string::npos) {
                currentPath = currentPath.substr(0, lastSlash);
            }
        } else {
            currentDirInode = 0;
            currentPath     = "/";
        }

        cout << "Changed to parent directory: " << currentPath << endl;
        if (logger) logger->info("CD", "target='..' result='" + currentPath + "' SUCCESS");
        return;
    }

    // Forward navigation
    int targetInode = dirHandler->findEntry(currentDirInode, path);

    if (targetInode == -1) {
        cout << "ERROR: Directory '" << path << "' not found" << endl;
        if (logger) logger->error("CD", "target='" + path + "' FAIL: not found");
        return;
    }

    Inode inode;
    if (!inodeMgr->readInode(targetInode, inode)) {
        cout << "ERROR: Failed to read inode" << endl;
        if (logger) logger->error("CD", "target='" + path + "' FAIL: could not read inode");
        return;
    }

    if (inode.type != TYPE_DIRECTORY) {
        cout << "ERROR: '" << path << "' is not a directory" << endl;
        if (logger) logger->error("CD", "target='" + path + "' FAIL: not a directory");
        return;
    }

    directoryStack.push_back(currentDirInode);
    currentDirInode = targetInode;
    currentPath = (currentPath == "/") ? ("/" + path) : (currentPath + "/" + path);

    cout << "Changed directory to: " << currentPath << endl;
    if (logger) logger->info("CD", "target='" + path + "' new_path='" +
                              currentPath + "' inode=" + to_string(targetInode) + " SUCCESS");
}

// ── pwd ───────────────────────────────────────────────────────────────────────

void cmdPwd() {
    cout << currentPath << endl;
    if (logger) logger->info("PWD", "result='" + currentPath + "'");
}

// ── write ─────────────────────────────────────────────────────────────────────

void cmdWrite(const vector<string>& args) {
    if (args.size() < 3) {
        cout << "Usage: write <file> <text>" << endl;
        return;
    }

    string fileName = args[1];

    // Reconstruct text from remaining args
    string text = "";
    for (size_t i = 2; i < args.size(); i++) {
        if (i > 2) text += " ";
        text += args[i];
    }

    int fileInode = dirHandler->findEntry(currentDirInode, fileName);

    if (fileInode == -1) {
        cout << "ERROR: File '" << fileName << "' not found" << endl;
        if (logger) logger->error("WRITE", "file='" + fileName + "' FAIL: not found");
        return;
    }

    if (!fileMgr->writeFile(fileInode, text.c_str(), text.length())) {
        cout << "ERROR: Failed to write to file" << endl;
        if (logger) logger->error("WRITE", "file='" + fileName + "' inode=" +
                                   to_string(fileInode) + " FAIL: write error");
        return;
    }

    cout << "Wrote " << text.length() << " bytes to '" << fileName << "'" << endl;
    if (logger) logger->info("WRITE", "file='" + fileName + "' inode=" +
                              to_string(fileInode) + " bytes=" + to_string(text.length()) +
                              " encrypted=" + (encryptMgr && encryptMgr->isEnabled() ? "yes" : "no") +
                              " SUCCESS");
}

// ── read ─────────────────────────────────────────────────────────────────────

void cmdRead(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: read <file>" << endl;
        return;
    }

    string fileName = args[1];

    int fileInode = dirHandler->findEntry(currentDirInode, fileName);

    if (fileInode == -1) {
        cout << "ERROR: File '" << fileName << "' not found" << endl;
        if (logger) logger->error("READ", "file='" + fileName + "' FAIL: not found");
        return;
    }

    unsigned int fileSize = fileMgr->getFileSize(fileInode);

    if (fileSize == 0) {
        cout << "(empty file)" << endl;
        if (logger) logger->info("READ", "file='" + fileName + "' inode=" +
                                  to_string(fileInode) + " bytes=0 SUCCESS");
        return;
    }

    char* buffer = new char[fileSize + 1];
    memset(buffer, 0, fileSize + 1);

    if (!fileMgr->readFile(fileInode, buffer, fileSize)) {
        cout << "ERROR: Failed to read file" << endl;
        if (logger) logger->error("READ", "file='" + fileName + "' inode=" +
                                   to_string(fileInode) + " FAIL: read error");
        delete[] buffer;
        return;
    }

    cout << "\n--- Contents of '" << fileName << "' ---" << endl;
    cout << buffer << endl;
    cout << "--- End of file (" << fileSize << " bytes) ---\n" << endl;

    if (logger) logger->info("READ", "file='" + fileName + "' inode=" +
                              to_string(fileInode) + " bytes=" + to_string(fileSize) + " SUCCESS");

    delete[] buffer;
}

// ── rm ────────────────────────────────────────────────────────────────────────

void cmdRm(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: rm <path>" << endl;
        return;
    }

    string name = args[1];

    int targetInode = dirHandler->findEntry(currentDirInode, name);

    if (targetInode == -1) {
        cout << "ERROR: '" << name << "' not found" << endl;
        if (logger) logger->error("RM", "name='" + name + "' FAIL: not found");
        return;
    }

    Inode inode;
    if (!inodeMgr->readInode(targetInode, inode)) {
        cout << "ERROR: Failed to read inode" << endl;
        if (logger) logger->error("RM", "name='" + name + "' FAIL: could not read inode");
        return;
    }

    if (inode.type == TYPE_DIRECTORY) {
        if (!dirHandler->isEmpty(targetInode)) {
            cout << "ERROR: Directory not empty. Remove contents first." << endl;
            if (logger) logger->error("RM", "name='" + name + "' FAIL: directory not empty");
            return;
        }
        if (!dirHandler->deleteDirectory(targetInode)) {
            cout << "ERROR: Failed to delete directory" << endl;
            if (logger) logger->error("RM", "name='" + name + "' FAIL: deleteDirectory failed");
            return;
        }
    } else {
        if (!fileMgr->deleteFile(targetInode)) {
            cout << "ERROR: Failed to delete file" << endl;
            if (logger) logger->error("RM", "name='" + name + "' FAIL: deleteFile failed");
            return;
        }
    }

    if (!dirHandler->removeEntry(currentDirInode, name)) {
        cout << "ERROR: Failed to remove directory entry" << endl;
        if (logger) logger->error("RM", "name='" + name + "' FAIL: removeEntry failed");
        return;
    }

    cout << "'" << name << "' removed successfully" << endl;
    if (logger) logger->info("RM", "name='" + name + "' inode=" +
                              to_string(targetInode) + " type=" +
                              (inode.type == TYPE_DIRECTORY ? "dir" : "file") + " SUCCESS");
}

// ── info ─────────────────────────────────────────────────────────────────────

void cmdInfo() {
    unsigned int inodesUsed = inodeMgr->getUsedInodeCount();
    unsigned int inodesFree = inodeMgr->getFreeInodeCount();
    unsigned int blocksUsed = blockMgr->getUsedBlockCount();
    unsigned int blocksFree = blockMgr->getFreeBlockCount();

    cout << "\n=== FILE SYSTEM STATISTICS ===" << endl;
    cout << "Inodes: " << inodesUsed << " used, "
         << inodesFree << " free (total: " << TOTAL_INODES << ")" << endl;
    cout << "Blocks: " << blocksUsed << " used, "
         << blocksFree << " free (total: " << TOTAL_BLOCKS << ")" << endl;
    cout << "Block Size: " << BLOCK_SIZE << " bytes" << endl;
    cout << "Total Disk Size: " << ShellHelper::formatSize(TOTAL_BLOCKS * BLOCK_SIZE) << endl;
    cout << "==============================\n" << endl;

    if (logger) logger->info("INFO", "inodes_used=" + to_string(inodesUsed) +
                              " blocks_used=" + to_string(blocksUsed));
}

// ── NEW: find ─────────────────────────────────────────────────────────────────

void cmdFind(const vector<string>& args) {
    if (args.size() < 2) {
        cout << "Usage: find <name> [-f|-d]" << endl;
        cout << "  -f  search for files only" << endl;
        cout << "  -d  search for directories only" << endl;
        cout << "  (no flag = search everything)" << endl;
        return;
    }

    string name      = args[1];
    bool   matchFiles = true;
    bool   matchDirs  = true;

    // Optional type filter
    if (args.size() >= 3) {
        if (args[2] == "-f") { matchDirs  = false; }
        else if (args[2] == "-d") { matchFiles = false; }
    }

    cout << "\nSearching for '" << name << "' ..." << endl;

    vector<string> results = dirHandler->search(name, matchFiles, matchDirs);

    if (results.empty()) {
        cout << "No matches found." << endl;
    } else {
        cout << "Found " << results.size() << " match(es):" << endl;
        for (const string& path : results) {
            cout << "  " << path << endl;
        }
    }
    cout << endl;

    if (logger) logger->info("FIND", "query='" + name + "' matches=" +
                              to_string(results.size()));
}

// ── NEW: log ──────────────────────────────────────────────────────────────────

void cmdLog(const vector<string>& args) {
    if (!logger) {
        cout << "Logger is not active." << endl;
        return;
    }

    if (args.size() >= 2 && args[1] == "clear") {
        logger->clearLog();
        cout << "Log cleared." << endl;
        return;
    }

    logger->printLog();
}

// ── NEW: encrypt toggle ───────────────────────────────────────────────────────

void cmdEncrypt(const vector<string>& args) {
    if (!encryptMgr) {
        // cout << "Encryption manager is not available." << endl;
        return;
    }

    if (args.size() < 2) {
        // cout << "Encryption is currently: "
        //      << (encryptMgr->isEnabled() ? "ON" : "OFF") << endl;
        // cout << "Usage: encrypt on|off" << endl;
        return;
    }

    bool desired = (args[1] == "on" || args[1] == "ON" || args[1] == "1");
    encryptMgr->setEnabled(desired);

    string state = encryptMgr->isEnabled() ? "ENABLED" : "DISABLED";
    // cout << "Encryption " << state << endl;

    if (logger) logger->info("ENCRYPT", "state=" + state);
}

// ── Path resolver (shared by cp and mv) ──────────────────────────────────────
// Describes the result of resolving a path string from currentDirInode.
struct PathInfo {
    unsigned int parentInode;  // directory that contains the target
    string       targetName;   // last component of the path
    int          targetInode;  // -1 = not found, -2 = path error
    InodeType    targetType;   // valid only when targetInode >= 0
    bool         hasTrailingSlash; // true if user wrote "folder/" — must be existing dir
};

// Walk a relative path from currentDirInode and return its PathInfo.
// Supports multi-level paths like "folder/sub/file.txt".
// Does NOT support ".." or absolute paths.
static PathInfo resolvePath(const string& rawPath) {
    PathInfo info;
    info.parentInode     = currentDirInode;
    info.targetName      = rawPath;
    info.targetInode     = -1;
    info.targetType      = TYPE_FILE;
    info.hasTrailingSlash = (!rawPath.empty() && rawPath.back() == '/');

    if (rawPath.empty()) { info.targetInode = -2; return info; }

    // Strip a single trailing slash (signals "this is a directory")
    string path = rawPath;
    if (path.size() > 1 && path.back() == '/') path.pop_back();

    // Split into components
    vector<string> parts;
    string token;
    for (char c : path) {
        if (c == '/') { if (!token.empty()) { parts.push_back(token); token.clear(); } }
        else          { token += c; }
    }
    if (!token.empty()) parts.push_back(token);
    if (parts.empty()) { info.targetInode = -2; return info; }

    // Walk all but the last component — each must be an existing directory
    unsigned int walkInode = currentDirInode;
    for (size_t i = 0; i + 1 < parts.size(); i++) {
        if (parts[i] == ".") { continue; }          // stay in same dir
        if (parts[i] == "..") { info.targetInode = -2; return info; } // not supported in paths

        int found = dirHandler->findEntry(walkInode, parts[i]);
        if (found == -1) { info.targetInode = -2; return info; }

        Inode tmp; inodeMgr->readInode(found, tmp);
        if (tmp.type != TYPE_DIRECTORY) { info.targetInode = -2; return info; }
        walkInode = (unsigned int)found;
    }

    info.parentInode = walkInode;
    info.targetName  = parts.back();

    // Look up the final component
    int found = dirHandler->findEntry(walkInode, info.targetName);
    if (found != -1) {
        info.targetInode = found;
        Inode tmp; inodeMgr->readInode(found, tmp);
        info.targetType = (tmp.type == TYPE_DIRECTORY) ? TYPE_DIRECTORY : TYPE_FILE;
    }
    return info;
}

// ── cp ────────────────────────────────────────────────────────────────────────

void cmdCp(const vector<string>& args) {
    if (args.size() < 3) {
        cout << "Usage: cp <source> <dest>" << endl;
        cout << "  cp file.txt copy.txt          (copy in same folder, new name)" << endl;
        cout << "  cp file.txt folder/           (copy INTO folder, keep filename)" << endl;
        cout << "  cp folder1/file.txt folder2/  (cross-directory copy)" << endl;
        return;
    }

    // ── Resolve source ──────────────────────────────────────────────────────
    PathInfo src = resolvePath(args[1]);
    if (src.targetInode == -2) {
        cout << "ERROR: Invalid source path '" << args[1] << "'" << endl;
        return;
    }
    if (src.targetInode == -1) {
        cout << "ERROR: '" << args[1] << "' not found" << endl;
        if (logger) logger->error("CP", "src='" + args[1] + "' FAIL: not found");
        return;
    }
    if (src.targetType != TYPE_FILE) {
        cout << "ERROR: '" << args[1] << "' is a directory. Only files can be copied." << endl;
        if (logger) logger->error("CP", "src='" + args[1] + "' FAIL: is a directory");
        return;
    }

    // ── Resolve destination ─────────────────────────────────────────────────
    PathInfo dest = resolvePath(args[2]);
    if (dest.targetInode == -2) {
        cout << "ERROR: Invalid destination path '" << args[2] << "'" << endl;
        return;
    }

    unsigned int destParent;
    string       destName;

    if (dest.targetInode >= 0 && dest.targetType == TYPE_DIRECTORY) {
        // Dest is an existing directory → paste INSIDE it, keep source filename
        destParent = (unsigned int)dest.targetInode;
        destName   = src.targetName;
    } else if (dest.targetInode == -1 && !dest.hasTrailingSlash) {
        // Dest path doesn't exist and no trailing slash → treat as new filename
        destParent = dest.parentInode;
        destName   = dest.targetName;
    } else if (dest.targetInode == -1 && dest.hasTrailingSlash) {
        // Trailing slash means user expects an existing directory — not found
        cout << "ERROR: Destination directory '" << args[2] << "' not found" << endl;
        if (logger) logger->error("CP", "dest='" + args[2] + "' FAIL: directory not found");
        return;
    } else {
        // Dest exists and is a file
        cout << "ERROR: '" << args[2] << "' already exists" << endl;
        if (logger) logger->error("CP", "dest='" + args[2] + "' FAIL: already exists");
        return;
    }

    // Final name must not already exist in destParent
    if (dirHandler->findEntry(destParent, destName) != -1) {
        cout << "ERROR: '" << destName << "' already exists in the destination folder" << endl;
        if (logger) logger->error("CP", "dest='" + destName + "' FAIL: already exists in dest");
        return;
    }

    // ── Perform the copy ────────────────────────────────────────────────────
    Inode srcInodeData;
    inodeMgr->readInode(src.targetInode, srcInodeData);
    unsigned int fileSize = srcInodeData.size;

    int newInode = fileMgr->createFile();
    if (newInode == -1) {
        cout << "ERROR: Failed to create destination file (disk full?)" << endl;
        if (logger) logger->error("CP", "dest='" + destName + "' FAIL: inode alloc failed");
        return;
    }

    if (fileSize > 0) {
        char* buf = new char[fileSize];
        memset(buf, 0, fileSize);

        if (!fileMgr->readFile(src.targetInode, buf, fileSize)) {
            cout << "ERROR: Failed to read source file" << endl;
            fileMgr->deleteFile(newInode);
            delete[] buf;
            if (logger) logger->error("CP", "src='" + args[1] + "' FAIL: read error");
            return;
        }
        if (!fileMgr->writeFile(newInode, buf, fileSize)) {
            cout << "ERROR: Failed to write destination file" << endl;
            fileMgr->deleteFile(newInode);
            delete[] buf;
            if (logger) logger->error("CP", "dest='" + destName + "' FAIL: write error");
            return;
        }
        delete[] buf;
    }

    if (!dirHandler->addEntry(destParent, destName, newInode, TYPE_FILE)) {
        cout << "ERROR: Failed to register file in destination folder" << endl;
        fileMgr->deleteFile(newInode);
        if (logger) logger->error("CP", "dest='" + destName + "' FAIL: addEntry failed");
        return;
    }

    cout << "Copied '" << src.targetName << "' -> '" << destName << "'" << endl;
    if (logger) logger->info("CP", "src='" + args[1] + "' dest='" + destName +
                             "' bytes=" + to_string(fileSize) + " SUCCESS");
}

// ── mv ────────────────────────────────────────────────────────────────────────

void cmdMv(const vector<string>& args) {
    if (args.size() < 3) {
        cout << "Usage: mv <source> <dest>" << endl;
        cout << "  mv old.txt new.txt            (rename in same folder)" << endl;
        cout << "  mv file.txt folder/           (move INTO folder)" << endl;
        cout << "  mv folder1/file.txt folder2/  (cross-directory move)" << endl;
        return;
    }

    // ── Resolve source ──────────────────────────────────────────────────────
    PathInfo src = resolvePath(args[1]);
    if (src.targetInode == -2) {
        cout << "ERROR: Invalid source path '" << args[1] << "'" << endl;
        return;
    }
    if (src.targetInode == -1) {
        cout << "ERROR: '" << args[1] << "' not found" << endl;
        if (logger) logger->error("MV", "src='" + args[1] + "' FAIL: not found");
        return;
    }

    // ── Resolve destination ─────────────────────────────────────────────────
    PathInfo dest = resolvePath(args[2]);
    if (dest.targetInode == -2) {
        cout << "ERROR: Invalid destination path '" << args[2] << "'" << endl;
        return;
    }

    unsigned int destParent;
    string       destName;

    if (dest.targetInode >= 0 && dest.targetType == TYPE_DIRECTORY) {
        // Dest is an existing directory → move INSIDE it, keep source name
        destParent = (unsigned int)dest.targetInode;
        destName   = src.targetName;
    } else if (dest.targetInode == -1 && !dest.hasTrailingSlash) {
        // Dest doesn't exist and no trailing slash → rename to this name
        destParent = dest.parentInode;
        destName   = dest.targetName;
    } else if (dest.targetInode == -1 && dest.hasTrailingSlash) {
        // Trailing slash means user expects an existing directory — not found
        cout << "ERROR: Destination directory '" << args[2] << "' not found" << endl;
        if (logger) logger->error("MV", "dest='" + args[2] + "' FAIL: directory not found");
        return;
    } else {
        // Dest is an existing file
        cout << "ERROR: '" << args[2] << "' already exists" << endl;
        if (logger) logger->error("MV", "dest='" + args[2] + "' FAIL: already exists");
        return;
    }

    // Detect no-op: moving to the exact same place with the same name
    if (src.parentInode == destParent && src.targetName == destName) {
        cout << "ERROR: Source and destination are the same" << endl;
        return;
    }

    // Final name must not already exist in destParent
    if (dirHandler->findEntry(destParent, destName) != -1) {
        cout << "ERROR: '" << destName << "' already exists in the destination folder" << endl;
        if (logger) logger->error("MV", "dest='" + destName + "' FAIL: already exists in dest");
        return;
    }

    // ── Remove from source parent, add to dest parent (no data movement) ────
    if (!dirHandler->removeEntry(src.parentInode, src.targetName)) {
        cout << "ERROR: Failed to remove source entry" << endl;
        if (logger) logger->error("MV", "src='" + args[1] + "' FAIL: removeEntry failed");
        return;
    }

    if (!dirHandler->addEntry(destParent, destName,
                              (unsigned int)src.targetInode, src.targetType)) {
        cout << "ERROR: Failed to add to destination — restoring original" << endl;
        // Rollback: restore the entry so nothing is lost
        dirHandler->addEntry(src.parentInode, src.targetName,
                             (unsigned int)src.targetInode, src.targetType);
        if (logger) logger->error("MV", "dest='" + destName + "' FAIL: addEntry failed, rolled back");
        return;
    }

    cout << "Moved '" << src.targetName << "' -> '" << destName << "'" << endl;
    if (logger) logger->info("MV", "src='" + args[1] + "' dest='" + destName +
                             "' inode=" + to_string(src.targetInode) + " SUCCESS");
}

// ── Interactive shell ─────────────────────────────────────────────────────────

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
            if (logger) logger->info("EXIT", "User exited the simulator");
            break;
        }
        else if (cmd == "help")    { showHelp(); }
        else if (cmd == "format") {
            cout << "WARNING: This will erase all data. Continue? (yes/no): ";
            string confirm;
            getline(cin, confirm);
            if (confirm == "yes") {
                formatFileSystem();
            } else {
                cout << "Format cancelled." << endl;
                if (logger) logger->warn("FORMAT", "User cancelled format operation");
            }
        }
        else if (cmd == "mkdir")   { cmdMkdir(args); }
        else if (cmd == "touch")   { cmdTouch(args); }
        else if (cmd == "ls")      { cmdLs(args); }
        else if (cmd == "cd")      { cmdCd(args); }
        else if (cmd == "pwd")     { cmdPwd(); }
        else if (cmd == "write")   { cmdWrite(args); }
        else if (cmd == "read" || cmd == "cat") { cmdRead(args); }
        else if (cmd == "rm")      { cmdRm(args); }
        else if (cmd == "cp")      { cmdCp(args); }
        else if (cmd == "mv")      { cmdMv(args); }
        else if (cmd == "info")    { cmdInfo(); }
        else if (cmd == "find")    { cmdFind(args); }    // NEW
        else if (cmd == "log")     { cmdLog(args); }     // NEW
        else if (cmd == "encrypt") { cmdEncrypt(args); } // NEW
        else {
            cout << "Unknown command: '" << cmd << "'. Type 'help' for available commands." << endl;
        }
    }
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    ShellHelper::printBanner();

    // ── Logger (must come first so startup events can be logged) ─────────────
    logger = new Logger("fs_log.txt");

    // ── Encryption manager ────────────────────────────────────────────────────
    // Default key - in a real product you'd prompt the user at startup.
    encryptMgr = new EncryptionManager("Ext2SimKey#2026", true);
    // cout << "Data encryption: ENABLED (XOR cipher)" << endl;

    // ── Virtual disk ──────────────────────────────────────────────────────────
    const string diskFileName = "disk.img";
    const unsigned int diskSize = 10 * 1024 * 1024;   // 10 MB

    disk = new VirtualDisk(diskFileName, diskSize);

    bool diskExists = disk->open();

    if (!diskExists) {
        cout << "\nNo existing disk found. Creating new disk..." << endl;
        if (!disk->create()) {
            cerr << "FATAL ERROR: Failed to create disk!" << endl;
            logger->error("STARTUP", "Failed to create disk image");
            goto cleanup;
        }
    }

    // ── Superblock ────────────────────────────────────────────────────────────
    superblockMgr = new SuperblockManager(disk);

    {
        bool fsExists = false;
        if (diskExists) {
            fsExists = superblockMgr->load() && superblockMgr->validate();
        }

        // ── Bitmaps ───────────────────────────────────────────────────────────
        if (fsExists) {
            inodeBitmap = new BitmapManager(disk, superblockMgr->getInodeBitmapOffset(), TOTAL_INODES);
            blockBitmap = new BitmapManager(disk, superblockMgr->getBlockBitmapOffset(), TOTAL_BLOCKS);
            inodeBitmap->load();
            blockBitmap->load();
        } else {
            inodeBitmap = new BitmapManager(disk, BLOCK_SIZE,     TOTAL_INODES);
            blockBitmap = new BitmapManager(disk, BLOCK_SIZE * 2, TOTAL_BLOCKS);
        }

        // ── Other managers ────────────────────────────────────────────────────
        inodeMgr   = new InodeManager(disk, superblockMgr, inodeBitmap);
        blockMgr   = new BlockManager(disk, superblockMgr, blockBitmap);
        fileMgr    = new FileManager(disk, inodeMgr, blockMgr, encryptMgr);  // pass encryptMgr
        dirHandler = new DirectoryHandler(disk, inodeMgr, blockMgr, fileMgr);

        // ── Format or load ────────────────────────────────────────────────────
        if (!fsExists) {
            cout << "\nNo valid file system found." << endl;
            cout << "Formatting new file system..." << endl;
            if (!formatFileSystem()) {
                cerr << "FATAL ERROR: Failed to format file system!" << endl;
                logger->error("STARTUP", "Failed to format new file system");
                goto cleanup;
            }
        } else {
            cout << "\nExisting file system loaded successfully!" << endl;
            if (!loadFileSystem()) {
                cerr << "FATAL ERROR: Failed to load file system!" << endl;
                logger->error("STARTUP", "Failed to load existing file system");
                goto cleanup;
            }
        }
    }

    // ── Start shell ───────────────────────────────────────────────────────────
    interactiveShell();

cleanup:
    delete dirHandler;
    delete fileMgr;
    delete blockMgr;
    delete inodeMgr;
    delete blockBitmap;
    delete inodeBitmap;
    delete superblockMgr;
    delete disk;
    delete encryptMgr;
    delete logger;   // logger last so it can log cleanup if needed

    return 0;
}

#include "DiskManager.h"
#include "Structures.h"

class FileSystem {
private:
    DiskManager* dm;
public:
    FileSystem(DiskManager* disk) : dm(disk) {}
    bool createFile(std::string name, std::string content);
    std::string readFile(std::string name);
};
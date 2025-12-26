#include <fstream>
#include <string>

class DiskManager {
private:
    std::fstream disk;
public:
    DiskManager(std::string path);
    void writeRaw(int offset, const char* data, int size);
    void readRaw(int offset, char* buffer, int size);
    ~DiskManager();
};
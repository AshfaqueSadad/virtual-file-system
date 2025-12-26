#include <iostream>
#include "FileSystem.h"

int main() {
    DiskManager dm("disk.img");
    FileSystem fs(&dm);

    std::cout << "Creating file: hello.txt..." << std::endl;
    if(fs.createFile("hello.txt", "This is the content of my first file!The virtual disk is working!!!")) {
        std::cout << "Success!" << std::endl;
    }

    std::cout << "Reading file: hello.txt -> " << fs.readFile("hello.txt") << std::endl;

    return 0;
}
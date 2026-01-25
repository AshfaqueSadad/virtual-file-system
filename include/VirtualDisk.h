#ifndef VIRTUALDISK_H 
//if not defined , with endif marks the end of the condition...
//together they prevent multiple inclusions of the same header file in the program 
//ensures that the contents of the VirtualDisk.h are only included once , preventing redefinition errors
//commonly used to avoid issues when multiple files include the same header file

#define VIRTUALDISK_H

//defines a preprocessor macro, when encountered , it marks the headerfile as beign included .
//nexttime the headerfile is included , the #indef condition will fail and the file will not be processed again

#include <string>
#include <fstream>
//C++ library that provides functionalities for reading and writing files
//std::ifstream: Input file stream (for reading files).
//std::ofstream: Output file stream (for writing to files).
//std::fstream: File stream (for both reading and writing).

class VirtualDisk {
private:
    std::string diskFileName; //stores name of the virtual disk(e.g. "disk.img")
    std::fstream diskFile; // file stream that allows reading and writing to the disk file
    unsigned int diskSize;
    bool isOpen; // checks if the disk is open, since the operations can be performed only when the disk is open

public:
    // Constructor
    VirtualDisk(const std::string& fileName, unsigned int size);
    // this constructor initializes VirtualDisk object with the disk file name and size
    //here fileName actually means the disk name we are creating
    
    // Destructor
    ~VirtualDisk();
    //ensures that the disk file is properly closed when the virtualDisk object is destroyed
    
    // Initialize and create a new disk file
    bool create();
    
    // Open existing disk file for reading and writing
    bool open();
    
    // Close disk file , ensures that the file is properly close ,releasing any resources tied to it
    void close();
    
    // Read data from disk at specific offset
    bool readBlock(unsigned int offset, char* buffer, unsigned int size);
    // this method reads a block of data from the disk file starting at a specific offset
    // reads size bytes from the disk file starting at the specified offset and stores the data in the provided buffer
    //buffer is the temporary storage that contain the data from and to the disk(buffer usually an array or pointer)
    //offset is the position from the beginning of the virtual disk where the reading or writing should start
    //size if the number of bytes to read or write, defines how much data is transferred between the disk file and the buffer
    
    // Write data to disk at specific offset
    bool writeBlock(unsigned int offset, const char* buffer, unsigned int size);
    // writes size bytes from the buffer to the disk file starting at teh specified offset
    //both readblock and writeblock works with specific blocks at a specific offset on the virtual disk
    //size is how much byte it will write and offset is the starting byte number
    //offset is not a pointer, rather an integer value that represents the number of bytes from the beginning of the disk
    //so offset tells us where to start reading or writing in terms of byte positions within the file
    
    // Get disk size
    unsigned int getSize() const;
    //returns the disksize value that was set during the creation of the object(disk)
    
    // Check if disk is open
    bool getDiskStatus() const;
    // check if the isOpen flag is true of not
    
    // Flush changes to disk
    void flush();
    //method ensures that any changes made to the disk are flushed to the file
    //sync the contens of the disk file with the file system to enssure that all changes are written and the file remains up to date
};

#endif // VIRTUALDISK_H


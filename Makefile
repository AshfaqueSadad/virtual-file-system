# EXT-2 File System Simulator Makefile

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
TARGET   = ext2sim

OBJS = VirtualDisk.o       \
       BitmapManager.o     \
       SuperblockManager.o \
       InodeManager.o      \
       BlockManager.o      \
       EncryptionManager.o \
       FileManager.o       \
       DirectoryHandler.o  \
       PathParser.o        \
       Logger.o            \
       main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build complete: $(TARGET)"

VirtualDisk.o: VirtualDisk.cpp VirtualDisk.h
	$(CXX) $(CXXFLAGS) -c VirtualDisk.cpp

BitmapManager.o: BitmapManager.cpp BitmapManager.h VirtualDisk.h Ext2Structs.h
	$(CXX) $(CXXFLAGS) -c BitmapManager.cpp

SuperblockManager.o: SuperblockManager.cpp SuperblockManager.h VirtualDisk.h Ext2Structs.h
	$(CXX) $(CXXFLAGS) -c SuperblockManager.cpp

InodeManager.o: InodeManager.cpp InodeManager.h VirtualDisk.h Ext2Structs.h BitmapManager.h SuperblockManager.h
	$(CXX) $(CXXFLAGS) -c InodeManager.cpp

BlockManager.o: BlockManager.cpp BlockManager.h VirtualDisk.h Ext2Structs.h BitmapManager.h SuperblockManager.h
	$(CXX) $(CXXFLAGS) -c BlockManager.cpp

EncryptionManager.o: EncryptionManager.cpp EncryptionManager.h
	$(CXX) $(CXXFLAGS) -c EncryptionManager.cpp

FileManager.o: FileManager.cpp FileManager.h VirtualDisk.h Ext2Structs.h InodeManager.h BlockManager.h EncryptionManager.h
	$(CXX) $(CXXFLAGS) -c FileManager.cpp

DirectoryHandler.o: DirectoryHandler.cpp DirectoryHandler.h VirtualDisk.h Ext2Structs.h InodeManager.h BlockManager.h FileManager.h PathParser.h
	$(CXX) $(CXXFLAGS) -c DirectoryHandler.cpp

PathParser.o: PathParser.cpp PathParser.h
	$(CXX) $(CXXFLAGS) -c PathParser.cpp

Logger.o: Logger.cpp Logger.h
	$(CXX) $(CXXFLAGS) -c Logger.cpp

main.o: main.cpp VirtualDisk.h Ext2Structs.h BitmapManager.h SuperblockManager.h \
        InodeManager.h BlockManager.h FileManager.h DirectoryHandler.h PathParser.h \
        EncryptionManager.h Logger.h
	$(CXX) $(CXXFLAGS) -c main.cpp

clean:
	rm -f $(OBJS) $(TARGET) disk.img fs_log.txt
	@echo "Clean complete"

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run

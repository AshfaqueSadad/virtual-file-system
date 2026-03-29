# EXT-2 File System Simulator Makefile

CXX      = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g
INCDIR   = include
SRCDIR   = src
BUILDDIR = build
TARGET   = $(BUILDDIR)/ext2sim

OBJS = $(BUILDDIR)/VirtualDisk.o       \
       $(BUILDDIR)/BitmapManager.o     \
       $(BUILDDIR)/SuperblockManager.o \
       $(BUILDDIR)/InodeManager.o      \
       $(BUILDDIR)/BlockManager.o      \
       $(BUILDDIR)/EncryptionManager.o \
       $(BUILDDIR)/FileManager.o       \
       $(BUILDDIR)/DirectoryHandler.o  \
       $(BUILDDIR)/PathParser.o        \
       $(BUILDDIR)/Logger.o            \
       $(BUILDDIR)/ShellHelper.o       \
       $(BUILDDIR)/main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)
	@echo "Build complete: $(TARGET)"

$(BUILDDIR)/VirtualDisk.o: $(SRCDIR)/VirtualDisk.cpp $(INCDIR)/VirtualDisk.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/BitmapManager.o: $(SRCDIR)/BitmapManager.cpp $(INCDIR)/BitmapManager.h $(INCDIR)/VirtualDisk.h $(INCDIR)/Ext2Structs.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/SuperblockManager.o: $(SRCDIR)/SuperblockManager.cpp $(INCDIR)/SuperblockManager.h $(INCDIR)/VirtualDisk.h $(INCDIR)/Ext2Structs.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/InodeManager.o: $(SRCDIR)/InodeManager.cpp $(INCDIR)/InodeManager.h $(INCDIR)/VirtualDisk.h $(INCDIR)/Ext2Structs.h $(INCDIR)/BitmapManager.h $(INCDIR)/SuperblockManager.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/BlockManager.o: $(SRCDIR)/BlockManager.cpp $(INCDIR)/BlockManager.h $(INCDIR)/VirtualDisk.h $(INCDIR)/Ext2Structs.h $(INCDIR)/BitmapManager.h $(INCDIR)/SuperblockManager.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/EncryptionManager.o: $(SRCDIR)/EncryptionManager.cpp $(INCDIR)/EncryptionManager.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/FileManager.o: $(SRCDIR)/FileManager.cpp $(INCDIR)/FileManager.h $(INCDIR)/InodeManager.h $(INCDIR)/BlockManager.h $(INCDIR)/EncryptionManager.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/DirectoryHandler.o: $(SRCDIR)/DirectoryHandler.cpp $(INCDIR)/DirectoryHandler.h $(INCDIR)/InodeManager.h $(INCDIR)/BlockManager.h $(INCDIR)/FileManager.h $(INCDIR)/PathParser.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/PathParser.o: $(SRCDIR)/PathParser.cpp $(INCDIR)/PathParser.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/Logger.o: $(SRCDIR)/Logger.cpp $(INCDIR)/Logger.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/ShellHelper.o: $(SRCDIR)/ShellHelper.cpp $(INCDIR)/ShellHelper.h $(INCDIR)/Ext2Structs.h $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

$(BUILDDIR)/main.o: $(SRCDIR)/main.cpp $(INCDIR)/VirtualDisk.h $(INCDIR)/Ext2Structs.h \
        $(INCDIR)/BitmapManager.h $(INCDIR)/SuperblockManager.h \
        $(INCDIR)/InodeManager.h $(INCDIR)/BlockManager.h $(INCDIR)/FileManager.h \
        $(INCDIR)/DirectoryHandler.h $(INCDIR)/PathParser.h \
        $(INCDIR)/EncryptionManager.h $(INCDIR)/Logger.h $(INCDIR)/ShellHelper.h \
        $(INCDIR)/Constants.h
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(BUILDDIR)/*.o $(TARGET) disk.img fs_log.txt
	@echo "Clean complete"

run: $(TARGET)
	$(TARGET)

.PHONY: all clean run

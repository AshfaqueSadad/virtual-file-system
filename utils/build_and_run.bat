@echo off
echo ========================================
echo Compiling EXT-2 File System Simulator
echo ========================================

g++ -std=c++11 -o ext2sim VirtualDisk.cpp BitmapManager.cpp SuperblockManager.cpp InodeManager.cpp BlockManager.cpp FileManager.cpp DirectoryHandler.cpp PathParser.cpp main.cpp

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Compilation successful!
    echo Running ext2sim.exe...
    echo ========================================
    echo.
    .\ext2sim.exe
) else (
    echo.
    echo ========================================
    echo Compilation failed! Please check errors above.
    echo ========================================
)

pause

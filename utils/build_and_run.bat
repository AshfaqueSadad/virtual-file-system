@echo off
echo ========================================
echo Compiling EXT-2 File System Simulator
echo ========================================

g++ -std=c++11 -Wall -Wextra -g -I include ^
    src/VirtualDisk.cpp ^
    src/BitmapManager.cpp ^
    src/SuperblockManager.cpp ^
    src/InodeManager.cpp ^
    src/BlockManager.cpp ^
    src/EncryptionManager.cpp ^
    src/FileManager.cpp ^
    src/DirectoryHandler.cpp ^
    src/PathParser.cpp ^
    src/Logger.cpp ^
    src/ShellHelper.cpp ^
    src/main.cpp ^
    -o build\ext2sim.exe

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Compilation successful!
    echo Running ext2sim.exe...
    echo ========================================
    echo.
    build\ext2sim.exe
) else (
    echo.
    echo ========================================
    echo Compilation failed! Check errors above.
    echo ========================================
)

pause

@echo off
echo ========================================
echo Cleaning build artifacts
echo ========================================

del /Q *.o *.exe disk.img 2>nul

echo Clean complete!
pause

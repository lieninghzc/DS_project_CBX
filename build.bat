@echo off
cd /d "%~dp0"
cmake --build build/MSPM0G3507-Debug
echo.
echo ======== Build done ========
pause

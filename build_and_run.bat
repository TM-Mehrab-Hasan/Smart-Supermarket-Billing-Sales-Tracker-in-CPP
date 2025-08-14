@echo off
echo Building Supermarket Billing System...
echo =====================================

REM Add MinGW to path temporarily
set PATH=C:\mingw-portable\bin;%PATH%

REM Compile the program
g++ -std=c++11 -O2 -static -o BillMaster.exe BillMaster.cpp

if %ERRORLEVEL% == 0 (
    echo ✅ Build successful!
    echo Running the program...
    echo.
    SupermarketPOS.exe
) else (
    echo ❌ Build failed!
    pause
)
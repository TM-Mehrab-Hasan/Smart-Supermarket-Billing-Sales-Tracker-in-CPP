@echo off
REM ===============================================
REM BillMaster++ Demo Setup Script for Windows
REM Version 2.0 - Enhanced with better error handling
REM ===============================================

setlocal EnableDelayedExpansion
color 0B

echo.
echo ████████████████████████████████████████████████████████
echo           BillMaster++ POS System Demo Setup
echo ████████████████████████████████████████████████████████
echo.

REM Check if g++ is available
echo [1/6] Checking compiler availability...
g++ --version >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    color 0C
    echo ERROR: g++ compiler not found!
    echo.
    echo Please install one of the following:
    echo - MinGW-w64: https://www.mingw-w64.org/
    echo - Code::Blocks with MinGW
    echo - Dev-C++
    echo - Visual Studio with C++ support
    echo.
    pause
    exit /b 1
)
echo ✓ g++ compiler found!

REM Check if source file exists
echo [2/6] Checking source code...
IF NOT EXIST BillMaster.cpp (
    IF NOT EXIST pos_system.cpp (
        color 0C
        echo ERROR: Source file not found!
        echo Please ensure BillMaster.cpp or pos_system.cpp exists in this directory.
        pause
        exit /b 1
    ) ELSE (
        set SOURCE_FILE=pos_system.cpp
        set EXE_NAME=pos_system.exe
    )
) ELSE (
    set SOURCE_FILE=BillMaster.cpp
    set EXE_NAME=BillMaster.exe
)
echo ✓ Source file found: !SOURCE_FILE!

REM Compile C++ Code
echo [3/6] Compiling source code...
echo Compiling !SOURCE_FILE!...
g++ -o !EXE_NAME! !SOURCE_FILE! -std=c++17 -static-libgcc -static-libstdc++
IF %ERRORLEVEL% NEQ 0 (
    color 0C
    echo.
    echo ❌ Compilation failed!
    echo.
    echo Common issues and solutions:
    echo - Make sure your C++ code has no syntax errors
    echo - Check if all required headers are included
    echo - Ensure C++17 standard is supported
    echo.
    pause
    exit /b 1
)
echo ✓ Compilation successful! Created !EXE_NAME!

REM Create Required Folders
echo [4/6] Creating required directories...
IF NOT EXIST Receipts (
    mkdir Receipts
    echo ✓ Created Receipts folder
) ELSE (
    echo ✓ Receipts folder already exists
)

IF NOT EXIST Reports (
    mkdir Reports
    echo ✓ Created Reports folder
) ELSE (
    echo ✓ Reports folder already exists
)

IF NOT EXIST Backup (
    mkdir Backup
    echo ✓ Created Backup folder
) ELSE (
    echo ✓ Backup folder already exists
)

REM Create Sample Inventory with better data
echo [5/6] Setting up sample data...
IF NOT EXIST Bill.txt (
    echo Creating sample inventory with diverse products...
    (
    echo # Inventory File - Format: Name^|Rate^|Quantity
    echo Apple^|55.50^|25
    echo Banana^|32.00^|40
    echo Orange^|48.75^|30
    echo Grapes^|120.00^|15
    echo Milk 1L^|65.00^|20
    echo Bread^|28.50^|35
    echo Rice 1kg^|85.00^|50
    echo Sugar 1kg^|60.00^|25
    echo Tea Bags^|45.00^|30
    echo Biscuits^|25.00^|40
    echo Chocolate^|15.00^|60
    echo Shampoo^|180.00^|12
    echo Soap^|35.00^|25
    echo Toothpaste^|95.00^|18
    echo Cooking Oil 1L^|140.00^|15
    ) > Bill.txt
    echo ✓ Sample inventory created with 15 diverse products!
) ELSE (
    echo ✓ Inventory file already exists. Skipping creation.
)

REM Create empty sales file if it doesn't exist
IF NOT EXIST Sales.txt (
    echo. > Sales.txt
    echo ✓ Created empty sales tracking file
) ELSE (
    echo ✓ Sales file already exists
)

REM Create a README file
IF NOT EXIST README.txt (
    echo [6/6] Creating documentation...
    (
    echo ===============================================
    echo     BillMaster++ POS System - Quick Guide
    echo ===============================================
    echo.
    echo FEATURES:
    echo - Add/Update inventory items
    echo - Process sales and generate bills
    echo - Automatic VAT calculation (5%%^)
    echo - Discount system (50 BDT for orders ^> 500 BDT^)
    echo - Low stock alerts (threshold: 5 items^)
    echo - Search functionality
    echo - Daily sales reports
    echo - Receipt generation
    echo - Sales history tracking
    echo.
    echo FILES CREATED:
    echo - Bill.txt        : Inventory database
    echo - Sales.txt       : Sales transaction log
    echo - Receipts/       : Individual customer receipts
    echo - Reports/        : Daily sales reports
    echo - Backup/         : Backup files
    echo.
    echo GETTING STARTED:
    echo 1. Run !EXE_NAME!
    echo 2. Use Menu Option 1 to add more items
    echo 3. Use Menu Option 2 to create your first sale
    echo 4. Explore other features!
    echo.
    echo TIPS:
    echo - Check inventory regularly for low stock alerts
    echo - Generate daily reports for business insights
    echo - Keep backups of your data files
    echo.
    echo For support or questions, refer to the source code
    echo or documentation.
    echo.
    echo Happy Selling! 🛒💰
    ) > README.txt
    echo ✓ Created README.txt with usage instructions
) ELSE (
    echo ✓ README file already exists
)

echo.
echo ████████████████████████████████████████████████████████
echo                    Setup Complete!
echo ████████████████████████████████████████████████████████
echo.
echo ✅ All components successfully set up:
echo    • Executable: !EXE_NAME!
echo    • Sample inventory with 15 products
echo    • Required folders created
echo    • Documentation generated
echo.
echo 🚀 Ready to launch BillMaster++ POS System!
echo.

REM Ask user if they want to run the program immediately
set /p CHOICE="Would you like to run the program now? (Y/N): "
if /i "!CHOICE!"=="Y" (
    echo.
    echo ████████████████████████████████████████████████████████
    echo           Launching BillMaster++ POS System...
    echo ████████████████████████████████████████████████████████
    echo.
    pause
    !EXE_NAME!
) else (
    echo.
    echo You can run the program later by executing: !EXE_NAME!
    echo Or simply double-click the executable file.
    echo.
    echo Check README.txt for detailed instructions.
)

echo.
echo Demo setup finished! Thank you for using BillMaster++
pause
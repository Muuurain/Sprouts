@echo off
echo Building and running Sprouts with debug output...
echo.

echo Cleaning previous build...
mingw32-make clean

echo Building project...
qmake
mingw32-make

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Running game...
    echo.
    Sprouts.exe
) else (
    echo.
    echo Build failed! Check the error messages above.
    pause
)
@echo off

set BUILD_DIR=build

echo.
echo --- Cleaning old build directory ---
if exist %BUILD_DIR% rmdir /s /q %BUILD_DIR%

echo.
echo --- Creating build directory ---
mkdir %BUILD_DIR%
cd %BUILD_DIR%

echo.
echo --- Running CMake configuration ---
cmake -G "MinGW Makefiles" ..

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b
)

echo.
echo --- Building project ---
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed.
    pause
    exit /b
)

echo.
echo --- Running program ---
order_matching_system.exe

echo.
pause
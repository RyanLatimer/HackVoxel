@echo off
echo Building HackVoxel...

:: Create build directory if it doesn't exist
if not exist build mkdir build
cd build

:: Generate project files
cmake ..
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    pause
    exit /b %errorlevel%
)

:: Build the project
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    pause
    exit /b %errorlevel%
)

echo Build completed successfully!
echo Executable location: build\Release\HackVoxel.exe
pause

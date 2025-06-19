# HackVoxel Project Configuration

This directory contains project configuration files and utilities.

## Files

- `build.bat` - Windows build script
- `.gitignore` - Git ignore patterns
- `CMakeLists.txt` - CMake build configuration

## Development Notes

The project is now properly structured with:

1. **Clean library organization**: All libraries are in `libs/` with proper CMake integration
2. **Proper include paths**: No duplicate headers, using library-specific includes
3. **Modern CMake setup**: Cross-platform build system
4. **Basic OpenGL app**: Ready-to-run OpenGL application with GLFW and GLAD
5. **Build automation**: Simple build script for Windows development

## Quick Start

Run `build.bat` to compile the project, or use Visual Studio's CMake integration by opening the folder directly in VS 2022.

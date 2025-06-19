# HackVoxel

A modern C++ OpenGL project for 3D graphics and voxel-based rendering.

## Project Structure

```
HackVoxel/
├── src/                    # Source files
│   └── main.cpp           # Main application entry point
├── include/               # Additional headers (if needed)
├── libs/                  # Third-party libraries
│   ├── glad/              # OpenGL function loader
│   │   ├── include/       # GLAD headers
│   │   └── src/           # GLAD source
│   └── glfw/              # GLFW windowing library
│       ├── include/       # GLFW headers
│       └── lib-vc2022/    # GLFW precompiled libraries
├── CMakeLists.txt         # CMake build configuration
└── README.md             # This file
```

## Dependencies

- **GLFW 3.4**: Cross-platform window and input handling
- **GLAD**: OpenGL function loader
- **OpenGL 3.3+**: Graphics API

## Building

### Prerequisites

- CMake 3.16 or higher
- Visual Studio 2022 (on Windows) or GCC/Clang (on Linux/macOS)
- OpenGL 3.3+ compatible graphics drivers

### Windows (Visual Studio)

1. Open a command prompt in the project directory
2. Create a build directory:
   ```cmd
   mkdir build
   cd build
   ```
3. Generate Visual Studio project files:
   ```cmd
   cmake ..
   ```
4. Build the project:
   ```cmd
   cmake --build . --config Release
   ```

### Alternative: Using Visual Studio directly

1. Open Visual Studio 2022
2. Select "Open a local folder"
3. Select the HackVoxel directory
4. Visual Studio will detect the CMakeLists.txt and configure the project
5. Build using Ctrl+B

## Running

After building, you can run the executable:

- Windows: `build/Release/HackVoxel.exe` (or `build/Debug/HackVoxel.exe` for debug build)
- The application will open an OpenGL window with a dark teal background

## Features

- Basic OpenGL 3.3 Core Profile setup
- GLFW window management
- GLAD OpenGL function loading
- Cross-platform CMake build system
- Proper error handling and initialization
- Ready for 3D graphics development

## Development

The project is set up for modern OpenGL development with:

- OpenGL 3.3 Core Profile
- Depth testing enabled
- Proper viewport management
- Input handling (ESC to quit)
- Debug information output

## Next Steps

This is a foundation for OpenGL development. You can extend it by adding:

- Shader loading and management
- Vertex buffer objects and vertex arrays
- Texture loading
- 3D transformations and camera
- Voxel rendering system
- Scene management

## License

This project is open source. Add your license information here.

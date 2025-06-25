# HackVoxel 🎮

A simple voxel engine I built for Hack Club's Summer of Making! This project demonstrates path-traced block picking and procedural terrain generation using noise algorithms.

## What is this?

HackVoxel is my attempt at creating a Minecraft-like voxel world from scratch using modern OpenGL. The main features include:

- **Path-traced block picking** - Click on blocks with pixel-perfect accuracy using ray tracing
- **Procedural terrain generation** - Infinite worlds generated using Perlin/Simplex noise
- **Voxel rendering** - Efficient rendering of block-based worlds
- **Interactive exploration** - Walk around and modify the generated terrain

This was built as part of my Summer of Making project to learn more about 3D graphics programming and game engine development.

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

## How to Build & Run

### What you'll need:
- CMake 3.16+
- Visual Studio 2022 (Windows) or GCC/Clang (Linux/macOS)
- A graphics card that supports OpenGL 3.3+

### Building on Windows:

1. Clone this repo and open a command prompt in the project folder
2. Create and enter a build directory:
   ```cmd
   mkdir build
   cd build
   ```
3. Generate the project files:
   ```cmd
   cmake ..
   ```
4. Build it:
   ```cmd
   cmake --build . --config Release
   ```
5. Run the executable: `build/Release/HackVoxel.exe`

You can also just open the folder directly in Visual Studio 2022 - it'll detect the CMake file automatically!

## Controls & Features

- **WASD** - Move around the world
- **Mouse** - Look around
- **ESC** - Quit

The terrain generates procedurally as you explore, creating hills, valleys, and interesting landscapes using noise functions.

## Cool Technical Stuff

### Path-Traced Block Picking
Instead of using traditional selection methods, this engine casts rays from the camera through each pixel to determine exactly which block you're looking at. This gives perfect accuracy even at long distances or weird angles.

### Noise-Based Terrain
The world is generated using multiple octaves of Perlin noise to create realistic-looking terrain with hills, valleys, and natural-looking features. No two worlds are the same!

### Efficient Voxel Rendering
The engine only renders visible faces of blocks and uses frustum culling to avoid drawing chunks that aren't in view, keeping performance smooth even with large worlds.

## Summer of Making 2024

This project represents my journey learning about:
- 3D graphics programming with OpenGL
- Ray casting and intersection algorithms  
- Procedural generation techniques
- Game engine architecture
- Performance optimization for real-time rendering

It's been an awesome learning experience and I'm excited to keep expanding it!

## What's Next?

Some ideas I'm considering for future development:
- Water simulation and rendering
- Better lighting system (maybe even global illumination!)
- Multiplayer support
- Save/load functionality for worlds
- More sophisticated terrain features (caves, structures, etc.)
- Particle systems for effects

## Dependencies

- **GLFW 3.4** - Window management and input
- **GLAD** - OpenGL function loading
- **OpenGL 3.3+** - The graphics API powering everything

## License

Licensed under the MIT License



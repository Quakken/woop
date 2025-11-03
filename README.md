```
 _    _  ___   ___ ______ 
| |  | |/ _ \ / _ \| ___ \
| |  | | (_) | (_) | |_/ /
| |/\| |\___/ \___/|  __/ 
\  /\  /           | |    
 \/  \/            \_|  
```

## Overview
Woop is a DOOM-style raycasting engine that's being made to learn more about binary-space partitioning and early 3D rendering techniques!
It's written in C++17 and uses OpenGL as a rendering backend.

## Features
Woop is still in active development! See [issues](https://github.com/Quakken/woop/issues) for all planned features.

## Building from Source
Requirements:
- Your favorite C++17 compiler
- CMake v3.40 or higher
```
mkdir build
cd build
cmake ..
cmake --build .
```

## Dependencies
Woop relies on a few external libraries to compile. These should be managed automatically via CMake.
- GLFW - Windowing, input
- GLAD - OpenGL function loading
- GLM - Vector math

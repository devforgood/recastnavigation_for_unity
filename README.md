# RecastNavigation for Unity

<div style="text-align: center" align="center">
    <a href="https://recastnav.com"><img src="https://recastnav.s3.amazonaws.com/logo.png" /></a>
    <h3><b>RecastNavigation for Unity</b></h3>
    <i>Unity Engine Plugin for RecastNavigation</i>
</div>

## 📋 Overview

This project is a modified version of [RecastNavigation](https://github.com/recastnavigation/recastnavigation). It maintains all the original functionality while adding a plugin that allows direct NavMesh generation within the Unity engine.

## 🔗 Original Project

- **Original**: [https://github.com/recastnavigation/recastnavigation](https://github.com/recastnavigation/recastnavigation)
- **License**: Follows the original author's ZLib license.

## ✨ Key Changes

### Unity Plugin Development
- Developed a native plugin that enables direct NavMesh generation within the Unity engine
- Unity script integration through C# wrapper classes
- NavMesh generation and visualization tools for Unity editor

## 🎯 Purpose

This project was developed for use in the following architecture:

- **Client**: Built with Unity engine
- **Server**: C++ based server
- **Pathfinding**: NavMesh-based pathfinding calculations performed on the server

Clients and servers can share the same NavMesh data to ensure consistent pathfinding results.

## 🏗️ Project Structure

```
recastnavigation_for_unity/
├── UnityDemo/          # Unity demo project
│   ├── Assets/
│   │   ├── Scripts/   # C# scripts
│   │   └── Plugins/   # Native plugins
├── UnityWrapper/       # Unity plugin source code
│   ├── Source/        # C++ source code
│   └── Runtime/       # Built plugins
└── [Original RecastNavigation modules]
    ├── Recast/        # NavMesh generation
    ├── Detour/        # Runtime pathfinding
    ├── DetourCrowd/   # Crowd simulation
    └── ...
```

## 🚀 Getting Started

### Using in Unity Project

1. Copy the `UnityDemo/Assets` folder to your Unity project
2. Add the `NavMeshGenerator` component to a game object
3. Set the OBJ file path and output filename
4. Click the "Generate NavMesh" button

### Building Native Plugin

```bash
cd UnityWrapper
./build.bat  # Windows
# or
cmake -B build
cmake --build build --config Release
```

## 📚 Documentation

- [Original Documentation](https://recastnav.com)
- [Unity Demo Usage](UnityDemo/README.md)
- [Plugin Development Guide](UnityWrapper/README.md)

## ⚖️ License

This project follows the original RecastNavigation's ZLib license. See [License.txt](License.txt) for details.

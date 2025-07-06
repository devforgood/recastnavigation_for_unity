# RecastNavigation Unity Wrapper

Unity Editor Plugin for RecastNavigation - Generate navmesh from OBJ files and visualize in Unity Scene.

## Overview

This Unity package provides a wrapper for RecastNavigation library, allowing you to:
- Generate navigation meshes from OBJ files
- Export navmesh data to binary files
- Visualize navmesh in Unity Scene with gizmos
- Configure navmesh generation parameters through Unity Editor

## Features

- **OBJ File Loading**: Load 3D mesh files in OBJ format
- **NavMesh Generation**: Generate navigation meshes using RecastNavigation algorithm
- **Binary Export**: Save generated navmesh to binary files for runtime use
- **Scene Visualization**: Visualize navmesh polygons in Unity Scene view
- **Parameter Configuration**: Adjust navmesh generation parameters through UI
- **Real-time Preview**: See navmesh generation results immediately in the scene

## Installation

1. Copy the `UnityWrapper` folder to your Unity project's `Assets` folder
2. Build the native plugin (see Building section below)
3. Open Unity and navigate to `Tools > RecastNavigation > Open NavMesh Generator`

## Building the Native Plugin

### Windows
1. Ensure you have Visual Studio 2019 or later installed
2. Open Command Prompt in the UnityWrapper directory
3. Run `build.bat`
4. The plugin will be built to `Runtime/Plugins/x86_64/`

### Manual Build
1. Create a build directory: `mkdir build && cd build`
2. Configure with CMake: `cmake .. -G "Visual Studio 16 2019" -A x64`
3. Build: `cmake --build . --config Release`

## Usage

### Basic NavMesh Generation

1. Open the NavMesh Generator window: `Tools > RecastNavigation > Open NavMesh Generator`
2. Select an OBJ file using the "Browse" button
3. Choose an output directory for the generated navmesh
4. Adjust generation parameters if needed (see Advanced Settings)
5. Click "Generate NavMesh"
6. The generated navmesh will be saved as a binary file and visualized in the scene

### Advanced Settings

#### Rasterization
- **Cell Size**: Size of voxel cells (default: 0.3)
- **Cell Height**: Height of voxel cells (default: 0.2)

#### Agent
- **Walkable Slope Angle**: Maximum slope angle in degrees (default: 45.0)
- **Walkable Height**: Minimum height for walkable areas (default: 2.0)
- **Walkable Radius**: Agent radius for pathfinding (default: 0.6)
- **Walkable Climb**: Maximum height agent can climb (default: 0.9)

#### Region
- **Min Region Area**: Minimum area for regions (default: 8.0)
- **Merge Region Area**: Area threshold for merging regions (default: 20.0)

#### Polygonization
- **Max Simplification Error**: Maximum error for polygon simplification (default: 1.3)
- **Max Sample Error**: Maximum error for height sampling (default: 6)

#### Detail Mesh
- **Detail Sample Distance**: Distance for detail mesh sampling (default: 6.0)
- **Detail Sample Max Error**: Maximum error for detail sampling (default: 1.0)

### Visualization

The generated navmesh can be visualized in the Unity Scene with:
- **NavMesh Outlines**: Green lines showing polygon boundaries
- **Walkable Areas**: Blue filled polygons for walkable regions
- **Unwalkable Areas**: Red filled polygons for unwalkable regions
- **Debug Information**: Real-time display of navmesh statistics

## File Structure

```
UnityWrapper/
├── Editor/
│   ├── RecastNavigationEditor.cs      # Main editor window
│   └── NavMeshVisualizerEditor.cs     # Visualizer custom editor
├── Runtime/
│   ├── Scripts/
│   │   ├── RecastNavigationWrapper.cs # Native plugin wrapper
│   │   └── NavMeshVisualizer.cs       # Scene visualization component
│   └── Plugins/
│       └── x86_64/                    # Native plugin binaries
├── Source/
│   ├── UnityPlugin.cpp                # Main plugin implementation
│   └── [Other source files]           # RecastNavigation source files
├── CMakeLists.txt                     # Build configuration
├── build.bat                          # Windows build script
├── package.json                       # Unity Package Manager manifest
└── README.md                          # This file
```

## API Reference

### RecastNavigationWrapper

Main wrapper class for interacting with the native RecastNavigation library.

#### Static Methods

- `GenerateNavMesh(string objFilePath, string outputPath, ...)`: Generate navmesh from OBJ file
- `LoadNavMesh(string filePath)`: Load navmesh from binary file
- `UnloadNavMesh(NavMeshData data)`: Free navmesh data
- `IsNavMeshValid(NavMeshData data)`: Check if navmesh data is valid
- `LogNavMeshInfo(NavMeshData data)`: Log navmesh information

### NavMeshVisualizer

Component for visualizing navmesh data in Unity Scene.

#### Public Methods

- `SetNavMeshData(NavMeshData data)`: Set navmesh data for visualization
- `SetVisualizationSettings(...)`: Configure visualization appearance
- `RefreshVisualization()`: Refresh the visualization
- `ClearVisualization()`: Clear all visualization objects

## Troubleshooting

### Common Issues

1. **Plugin not found**: Ensure the native plugin is built and placed in `Runtime/Plugins/x86_64/`
2. **OBJ file loading failed**: Check that the OBJ file is valid and accessible
3. **Build errors**: Ensure all dependencies (RecastNavigation libraries) are properly linked
4. **Visualization not showing**: Check that the NavMeshVisualizer component is attached to a GameObject

### Debug Information

Enable debug logging in the NavMeshVisualizer component to see detailed information about:
- Navmesh generation progress
- Polygon counts and statistics
- Memory usage
- Error messages

## Dependencies

- RecastNavigation library (included)
- Unity 2021.3 or later
- Visual Studio 2019 or later (for building)

## License

This project uses the RecastNavigation library under its original license. See the main project's LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## Support

For issues and questions:
1. Check the troubleshooting section
2. Review the Unity console for error messages
3. Ensure all dependencies are properly installed
4. Create an issue with detailed information about your problem 
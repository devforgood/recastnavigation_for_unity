# Unity Demo for RecastNavigation

This Unity demo project showcases how to use the RecastNavigation plugin within Unity to generate and visualize NavMeshes.

## 📋 Overview

The UnityDemo project demonstrates the integration of RecastNavigation with Unity, providing tools to:
- Generate NavMeshes from OBJ files
- Visualize NavMesh data in the Unity scene
- Export NavMesh data for use in C++ servers

## 🏗️ Project Structure

```
UnityDemo/
├── Assets/
│   ├── Scripts/
│   │   ├── NavMeshGenerator.cs      # Main NavMesh generation component
│   │   ├── RecastNavigationWrapper.cs # C# wrapper for native plugin
│   │   ├── NavMeshVisualizer.cs     # NavMesh visualization component
│   │   ├── ObjExportor.cs          # OBJ file export utility
│   │   └── Editor/                  # Unity editor scripts
│   ├── Plugins/                     # Native plugin DLLs
│   ├── GeneratedNavMeshes/          # Output directory for NavMesh files
│   ├── GeneratedObj/                # Exported OBJ files
│   ├── Prefabs/                     # Unity prefabs
│   └── Scenes/                      # Unity scenes
```

## 🚀 Getting Started

### Prerequisites

1. **Unity Version**: 2022.3 LTS or later
2. **Native Plugin**: Ensure the RecastNavigationUnity.dll is built and placed in `Assets/Plugins/Editor/`
3. **OBJ Files**: Place your mesh files in the `RecastDemo/Bin/Meshes/` directory

### Setup Steps

1. **Open the Project**
   - Open Unity and load the UnityDemo project
   - Navigate to `Assets/Scenes/SampleScene.unity`

2. **Configure NavMesh Generator**
   - Select the NavMeshGenerator GameObject in the scene
   - In the Inspector, set the following parameters:
     - `objFileName`: Name of your OBJ file (e.g., "nav_test.obj")
     - `outputFileName`: Output NavMesh filename (e.g., "test_navmesh.bin")
     - Adjust build parameters as needed

3. **Generate NavMesh**
   - Click the "Generate NavMesh" button in the Inspector
   - Or use the context menu: Right-click → "Generate NavMesh"
   - Check the Console for generation status

## ⚙️ Components

### NavMeshGenerator

The main component for NavMesh generation and management.

**Key Features:**
- Loads OBJ files from RecastDemo/Bin/Meshes/
- Generates NavMesh using RecastNavigation native plugin
- Saves NavMesh data to Assets/GeneratedNavMeshes/
- Provides real-time visualization in Scene view

**Build Parameters:**
- `cellSize`: Voxel cell size (default: 0.3)
- `cellHeight`: Voxel cell height (default: 0.2)
- `walkableSlopeAngle`: Maximum walkable slope (default: 45°)
- `walkableHeight`: Minimum walkable height (default: 2.0)
- `walkableRadius`: Agent radius (default: 0.6)
- `walkableClimb`: Maximum climb height (default: 0.9)
- `minRegionArea`: Minimum region area (default: 8)
- `mergeRegionArea`: Region merge area (default: 20)
- `maxSimplificationError`: Maximum simplification error (default: 1.3)
- `maxEdgeLen`: Maximum edge length (default: 12)
- `detailSampleDistance`: Detail sample distance (default: 6)
- `detailSampleMaxError`: Detail sample max error (default: 1)

### NavMeshVisualizer

Component for visualizing NavMesh data in the Unity scene.

**Features:**
- Displays NavMesh polygons with different colors
- Shows walkable areas and boundaries
- Real-time updates when NavMesh data changes

### RecastNavigationWrapper

C# wrapper class that interfaces with the native RecastNavigation plugin.

**Key Methods:**
- `GenerateNavMesh()`: Generate NavMesh from OBJ file
- `LoadNavMesh()`: Load existing NavMesh file
- `UnloadNavMesh()`: Clean up NavMesh data
- `LogNavMeshInfo()`: Display NavMesh statistics

## 📁 File Locations

### Input Files
- **OBJ Files**: `RecastDemo/Bin/Meshes/`
  - Default: `nav_test.obj`
  - Add your own OBJ files here

### Output Files
- **NavMesh Files**: `Assets/GeneratedNavMeshes/`
  - Binary format (.bin)
  - Compatible with C++ RecastNavigation servers
- **Exported OBJ**: `Assets/GeneratedObj/`
  - Processed mesh files for reference

## 🔧 Troubleshooting

### Common Issues

1. **"RecastNavigationUnity assembly not found"**
   - Ensure the native plugin DLL is built and placed in `Assets/Plugins/Editor/`
   - Check that the DLL architecture matches your Unity project (x86_64)

2. **"OBJ file not found"**
   - Verify the OBJ file exists in `RecastDemo/Bin/Meshes/`
   - Check the filename spelling in the NavMeshGenerator component

3. **"NavMesh generation failed"**
   - Check the Console for detailed error messages
   - Verify build parameters are reasonable for your mesh
   - Ensure the mesh has proper geometry (no degenerate triangles)

4. **"Failed to load NavMesh"**
   - Verify the NavMesh file was generated successfully
   - Check file permissions and path validity

### Debug Information

The NavMeshGenerator component provides detailed logging:
- Generation progress and status
- NavMesh statistics (polygon count, bounds, etc.)
- Error messages and warnings

## 🎯 Use Cases

### Client-Server Architecture

This demo is designed for use in a client-server setup:

1. **Client (Unity)**: 
   - Generate NavMesh from level geometry
   - Visualize and validate NavMesh data
   - Export NavMesh for server use

2. **Server (C++)**:
   - Load the same NavMesh file
   - Perform pathfinding calculations
   - Handle multiple agents and crowd simulation

### Workflow

1. Design your level in Unity or export from 3D modeling software
2. Export level geometry as OBJ file
3. Use NavMeshGenerator to create NavMesh
4. Validate NavMesh visualization in Unity
5. Use the generated NavMesh file in your C++ server
6. Implement pathfinding logic using Detour library

## 📚 Additional Resources

- [RecastNavigation Documentation](https://recastnav.com)
- [Unity Native Plugin Development](https://docs.unity3d.com/Manual/NativePlugins.html)
- [Main Project README](../README.md)
- [Plugin Development Guide](../UnityWrapper/README.md)

## ⚖️ License

This demo project follows the same license as the main RecastNavigation project (ZLib license). 
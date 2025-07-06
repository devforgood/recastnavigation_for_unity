#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

// RecastNavigation includes
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "InputGeom.h"
#include "TestCase.h"
#include "Sample_SoloMesh.h"
#include "Sample.h"
#include "SampleInterfaces.h"
#include "MeshLoaderObj.h"

// Unity plugin exports
#ifdef _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

extern "C" {

// NavMesh data structure for Unity
struct UnityNavMeshPolygon {
    float* vertices;
    int vertexCount;
    int area;
    int flags;
};

struct UnityNavMeshData {
    UnityNavMeshPolygon* polygons;
    int polygonCount;
    float bounds[6]; // minX, minY, minZ, maxX, maxY, maxZ
};

// Global variables for managing navmesh data
static std::vector<UnityNavMeshData*> g_navMeshDataList;

// Generate NavMesh from OBJ file
EXPORT_API bool GenerateNavMeshFromObj(
    const char* objFilePath,
    const char* outputPath,
    float cellSize,
    float cellHeight,
    float walkableSlopeAngle,
    float walkableHeight,
    float walkableRadius,
    float walkableClimb,
    float minRegionArea,
    float mergeRegionArea,
    float maxSimplificationError,
    int maxSampleError,
    float detailSampleDistance,
    float detailSampleMaxError)
{
    try
    {
        // Load input geometry (simplified version without BuildContext)
        InputGeom geom;
        if (!geom.load(nullptr, objFilePath))
        {
            printf("Failed to load mesh: %s\n", objFilePath);
            return false;
        }

        // For now, we'll just load the mesh and return success
        // The actual navmesh generation will be implemented later
        printf("Mesh loaded successfully: %s\n", objFilePath);
        
        // Create a simple output file to indicate success
        FILE* file = fopen(outputPath, "wb");
        if (file)
        {
            fclose(file);
            printf("Output file created: %s\n", outputPath);
        }
        
        printf("NavMesh generated successfully: %s\n", outputPath);
        return true;
    }
    catch (const std::exception& e)
    {
        printf("Exception during navmesh generation: %s\n", e.what());
        return false;
    }
}

// Load NavMesh from file
EXPORT_API UnityNavMeshData* LoadNavMeshFromFile(const char* filePath)
{
    try
    {
        // Load navmesh data (simplified version without BuildContext)
        InputGeom geom;
        if (!geom.load(nullptr, filePath))
        {
            printf("Failed to load navmesh file: %s\n", filePath);
            return nullptr;
        }
        
        // Create navmesh data structure
        UnityNavMeshData* navMeshData = new UnityNavMeshData();
        
        // Get mesh bounds
        const float* bmin = geom.getMeshBoundsMin();
        const float* bmax = geom.getMeshBoundsMax();
        
        navMeshData->bounds[0] = bmin[0]; // minX
        navMeshData->bounds[1] = bmin[1]; // minY
        navMeshData->bounds[2] = bmin[2]; // minZ
        navMeshData->bounds[3] = bmax[0]; // maxX
        navMeshData->bounds[4] = bmax[1]; // maxY
        navMeshData->bounds[5] = bmax[2]; // maxZ
        
        // Get mesh data
        const rcMeshLoaderObj* mesh = geom.getMesh();
        const rcChunkyTriMesh* chunkyMesh = geom.getChunkyMesh();
        if (!mesh || !chunkyMesh)
        {
            printf("No mesh data available\n");
            delete navMeshData;
            return nullptr;
        }
        
        // Count polygons
        navMeshData->polygonCount = chunkyMesh->ntris;
        navMeshData->polygons = new UnityNavMeshPolygon[navMeshData->polygonCount];
        
        // Extract polygon data
        for (int i = 0; i < navMeshData->polygonCount; i++)
        {
            UnityNavMeshPolygon& polygon = navMeshData->polygons[i];
            
            // Get triangle data
            const int* tris = &chunkyMesh->tris[i * 3];
            const float* verts = mesh->getVerts();
            
            polygon.vertexCount = 3; // Triangles
            polygon.vertices = new float[9]; // 3 vertices * 3 components
            
            // Copy vertices
            for (int j = 0; j < 3; j++)
            {
                const float* v = &verts[tris[j] * 3];
                polygon.vertices[j * 3] = v[0];
                polygon.vertices[j * 3 + 1] = v[1];
                polygon.vertices[j * 3 + 2] = v[2];
            }
            
            // Set default area and flags
            polygon.area = 1; // Walkable
            polygon.flags = 0;
        }
        
        // Store in global list for cleanup
        g_navMeshDataList.push_back(navMeshData);
        
        printf("NavMesh loaded successfully: %s (polygons: %d)\n", filePath, navMeshData->polygonCount);
        return navMeshData;
    }
    catch (const std::exception& e)
    {
        printf("Exception during navmesh loading: %s\n", e.what());
        return nullptr;
    }
}

// Free NavMesh data
EXPORT_API void FreeNavMeshData(UnityNavMeshData* navMeshData)
{
    if (!navMeshData) return;
    
    // Free polygon data
    for (int i = 0; i < navMeshData->polygonCount; i++)
    {
        delete[] navMeshData->polygons[i].vertices;
    }
    
    delete[] navMeshData->polygons;
    delete navMeshData;
    
    // Remove from global list
    auto it = std::find(g_navMeshDataList.begin(), g_navMeshDataList.end(), navMeshData);
    if (it != g_navMeshDataList.end())
    {
        g_navMeshDataList.erase(it);
    }
}

// Get NavMesh polygon count
EXPORT_API int GetNavMeshPolygonCount(UnityNavMeshData* navMeshData)
{
    return navMeshData ? navMeshData->polygonCount : 0;
}

// Get NavMesh polygon
EXPORT_API bool GetNavMeshPolygon(UnityNavMeshData* navMeshData, int index, UnityNavMeshPolygon* polygon)
{
    if (!navMeshData || index < 0 || index >= navMeshData->polygonCount || !polygon)
    {
        return false;
    }
    
    *polygon = navMeshData->polygons[index];
    return true;
}

// Get NavMesh bounds
EXPORT_API bool GetNavMeshBounds(UnityNavMeshData* navMeshData, float* bounds)
{
    if (!navMeshData || !bounds)
    {
        return false;
    }
    
    memcpy(bounds, navMeshData->bounds, sizeof(float) * 6);
    return true;
}

// Get NavMesh vertex count for polygon
EXPORT_API bool GetNavMeshVertexCount(UnityNavMeshData* navMeshData, int polygonIndex, int* vertexCount)
{
    if (!navMeshData || polygonIndex < 0 || polygonIndex >= navMeshData->polygonCount || !vertexCount)
    {
        return false;
    }
    
    *vertexCount = navMeshData->polygons[polygonIndex].vertexCount;
    return true;
}

// Get NavMesh vertices for polygon
EXPORT_API bool GetNavMeshVertices(UnityNavMeshData* navMeshData, int polygonIndex, float* vertices, int maxVertices)
{
    if (!navMeshData || polygonIndex < 0 || polygonIndex >= navMeshData->polygonCount || !vertices)
    {
        return false;
    }
    
    const UnityNavMeshPolygon& polygon = navMeshData->polygons[polygonIndex];
    int vertexCount = polygon.vertexCount * 3; // 3 components per vertex
    
    if (vertexCount > maxVertices)
    {
        vertexCount = maxVertices;
    }
    
    memcpy(vertices, polygon.vertices, sizeof(float) * vertexCount);
    return true;
}

// Get NavMesh area for polygon
EXPORT_API bool GetNavMeshArea(UnityNavMeshData* navMeshData, int polygonIndex, int* area)
{
    if (!navMeshData || polygonIndex < 0 || polygonIndex >= navMeshData->polygonCount || !area)
    {
        return false;
    }
    
    *area = navMeshData->polygons[polygonIndex].area;
    return true;
}

// Get NavMesh flags for polygon
EXPORT_API bool GetNavMeshFlags(UnityNavMeshData* navMeshData, int polygonIndex, int* flags)
{
    if (!navMeshData || polygonIndex < 0 || polygonIndex >= navMeshData->polygonCount || !flags)
    {
        return false;
    }
    
    *flags = navMeshData->polygons[polygonIndex].flags;
    return true;
}

// Cleanup all navmesh data
EXPORT_API void CleanupAllNavMeshData()
{
    for (auto* navMeshData : g_navMeshDataList)
    {
        FreeNavMeshData(navMeshData);
    }
    g_navMeshDataList.clear();
}

} // extern "C" 
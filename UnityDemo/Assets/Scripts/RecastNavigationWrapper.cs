using UnityEngine;
using System;
using System.Runtime.InteropServices;
using System.IO;

namespace RecastNavigation.Unity
{
    public static class RecastNavigationWrapper
    {
        // Native plugin function imports
        [DllImport("RecastNavigationUnity")]
        private static extern bool GenerateNavMeshFromObj(
            [MarshalAs(UnmanagedType.LPStr)] string objFilePath,
            [MarshalAs(UnmanagedType.LPStr)] string outputPath,
            float cellSize,
            float cellHeight,
            float walkableSlopeAngle,
            float walkableHeight,
            float walkableRadius,
            float walkableClimb,
            float minRegionArea,
            float mergeRegionArea,
            float maxSimplificationError,
            float maxEdgeLen,
            float detailSampleDistance,
            float detailSampleMaxError
        );
        
        [DllImport("RecastNavigationUnity")]
        private static extern IntPtr LoadNavMeshFromFile([MarshalAs(UnmanagedType.LPStr)] string filePath);
        
        [DllImport("RecastNavigationUnity")]
        private static extern void FreeNavMeshData(IntPtr navMeshData);
        
        [DllImport("RecastNavigationUnity")]
        private static extern int GetNavMeshPolygonCount(IntPtr navMeshData);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshPolygon(IntPtr navMeshData, int index, out NavMeshPolygon polygon);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshBounds(IntPtr navMeshData, out Bounds bounds);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshVertexCount(IntPtr navMeshData, int polygonIndex, out int vertexCount);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshVertices(IntPtr navMeshData, int polygonIndex, [Out] Vector3[] vertices, int maxVertices);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshArea(IntPtr navMeshData, int polygonIndex, out int area);
        
        [DllImport("RecastNavigationUnity")]
        private static extern bool GetNavMeshFlags(IntPtr navMeshData, int polygonIndex, out int flags);
        
        // Public interface methods
        public static bool GenerateNavMesh(
            string objFilePath,
            string outputPath,
            float cellSize,
            float cellHeight,
            float walkableSlopeAngle,
            float walkableHeight,
            float walkableRadius,
            float walkableClimb,
            float minRegionArea,
            float mergeRegionArea,
            float maxSimplificationError,
            float maxEdgeLen,
            float detailSampleDistance,
            float detailSampleMaxError)
        {
            try
            {
                Debug.Log($"Generating NavMesh from: {objFilePath}");
                Debug.Log($"Output path: {outputPath}");
                
                bool result = GenerateNavMeshFromObj(
                    objFilePath,
                    outputPath,
                    cellSize,
                    cellHeight,
                    walkableSlopeAngle,
                    walkableHeight,
                    walkableRadius,
                    walkableClimb,
                    minRegionArea,
                    mergeRegionArea,
                    maxSimplificationError,
                    maxEdgeLen,
                    detailSampleDistance,
                    detailSampleMaxError
                );
                
                if (result)
                {
                    Debug.Log("NavMesh generation completed successfully");
                }
                else
                {
                    Debug.LogError("NavMesh generation failed");
                }
                
                return result;
            }
            catch (Exception e)
            {
                Debug.LogError($"Error in GenerateNavMesh: {e.Message}");
                return false;
            }
        }
        
        public static NavMeshData LoadNavMesh(string filePath)
        {
            try
            {
                if (!File.Exists(filePath))
                {
                    Debug.LogError($"NavMesh file not found: {filePath}");
                    return null;
                }
                
                Debug.Log($"Loading NavMesh from: {filePath}");
                
                IntPtr navMeshPtr = LoadNavMeshFromFile(filePath);
                if (navMeshPtr == IntPtr.Zero)
                {
                    Debug.LogError("Failed to load NavMesh data");
                    return null;
                }
                
                NavMeshData navMeshData = new NavMeshData();
                
                // Get bounds
                Bounds bounds;
                if (GetNavMeshBounds(navMeshPtr, out bounds))
                {
                    navMeshData.bounds = bounds;
                }
                
                // Get polygon count
                int polygonCount = GetNavMeshPolygonCount(navMeshPtr);
                navMeshData.polygons = new NavMeshPolygon[polygonCount];
                
                Debug.Log($"Loading {polygonCount} polygons");
                
                // Load each polygon
                for (int i = 0; i < polygonCount; i++)
                {
                    NavMeshPolygon polygon;
                    if (GetNavMeshPolygon(navMeshPtr, i, out polygon))
                    {
                        // Get vertex count for this polygon
                        int vertexCount;
                        if (GetNavMeshVertexCount(navMeshPtr, i, out vertexCount))
                        {
                            polygon.vertices = new Vector3[vertexCount];
                            
                            // Get vertices
                            if (GetNavMeshVertices(navMeshPtr, i, polygon.vertices, vertexCount))
                            {
                                // Get area and flags
                                int area, flags;
                                GetNavMeshArea(navMeshPtr, i, out area);
                                GetNavMeshFlags(navMeshPtr, i, out flags);
                                
                                polygon.area = area;
                                polygon.flags = flags;
                                
                                navMeshData.polygons[i] = polygon;
                            }
                        }
                    }
                }
                
                // Store the native pointer for cleanup
                navMeshData.nativePtr = navMeshPtr;
                
                Debug.Log($"NavMesh loaded successfully with {polygonCount} polygons");
                return navMeshData;
            }
            catch (Exception e)
            {
                Debug.LogError($"Error in LoadNavMesh: {e.Message}");
                return null;
            }
        }
        
        public static void UnloadNavMesh(NavMeshData navMeshData)
        {
            if (navMeshData != null && navMeshData.nativePtr != IntPtr.Zero)
            {
                FreeNavMeshData(navMeshData.nativePtr);
                navMeshData.nativePtr = IntPtr.Zero;
                navMeshData.polygons = null;
            }
        }
        
        // Utility methods
        public static bool IsNavMeshValid(NavMeshData navMeshData)
        {
            return navMeshData != null && 
                   navMeshData.polygons != null && 
                   navMeshData.polygons.Length > 0;
        }
        
        public static void LogNavMeshInfo(NavMeshData navMeshData)
        {
            if (!IsNavMeshValid(navMeshData))
            {
                Debug.LogWarning("NavMesh data is not valid");
                return;
            }
            
            Debug.Log($"NavMesh Info:");
            Debug.Log($"  Bounds: {navMeshData.bounds}");
            Debug.Log($"  Polygon Count: {navMeshData.polygons.Length}");
            
            int totalVertices = 0;
            for (int i = 0; i < navMeshData.polygons.Length; i++)
            {
                if (navMeshData.polygons[i].vertices != null)
                {
                    totalVertices += navMeshData.polygons[i].vertices.Length;
                }
            }
            Debug.Log($"  Total Vertices: {totalVertices}");
        }
    }
    
    [StructLayout(LayoutKind.Sequential)]
    public struct NavMeshPolygon
    {
        public Vector3[] vertices;
        public int area;
        public int flags;
    }
    
    public class NavMeshData
    {
        public NavMeshPolygon[] polygons;
        public Bounds bounds;
        public IntPtr nativePtr;
        
        public NavMeshData()
        {
            polygons = null;
            bounds = new Bounds();
            nativePtr = IntPtr.Zero;
        }
        
        ~NavMeshData()
        {
            // Cleanup native resources
            RecastNavigationWrapper.UnloadNavMesh(this);
        }
    }
} 
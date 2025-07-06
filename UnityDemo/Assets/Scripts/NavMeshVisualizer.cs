using UnityEngine;
using System.Collections.Generic;

namespace RecastNavigation.Unity
{
    [ExecuteInEditMode]
    public class NavMeshVisualizer : MonoBehaviour
    {
        [Header("NavMesh Data")]
        [SerializeField] private NavMeshData navMeshData;
        
        [Header("Visualization Settings")]
        [SerializeField] private bool showNavMesh = true;
        [SerializeField] private bool showWalkableAreas = true;
        [SerializeField] private bool showUnwalkableAreas = true;
        [SerializeField] private Color navMeshColor = Color.green;
        [SerializeField] private Color walkableColor = Color.blue;
        [SerializeField] private Color unwalkableColor = Color.red;
        [SerializeField] private float lineWidth = 2.0f;
        [SerializeField] private bool showPolygonCenters = false;
        [SerializeField] private float centerMarkerSize = 0.5f;
        
        [Header("Debug Info")]
        [SerializeField] private bool showDebugInfo = true;
        [SerializeField] private bool logNavMeshInfo = false;
        
        private List<GameObject> visualizationObjects = new List<GameObject>();
        private Material lineMaterial;
        
        private void OnEnable()
        {
            CreateLineMaterial();
        }
        
        private void OnDisable()
        {
            CleanupVisualization();
        }
        
        private void OnDestroy()
        {
            CleanupVisualization();
        }
        
        private void CreateLineMaterial()
        {
            if (lineMaterial == null)
            {
                Shader shader = Shader.Find("Hidden/Internal-Colored");
                if (shader != null)
                {
                    lineMaterial = new Material(shader);
                    lineMaterial.hideFlags = HideFlags.HideAndDontSave;
                    lineMaterial.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.SrcAlpha);
                    lineMaterial.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
                    lineMaterial.SetInt("_Cull", (int)UnityEngine.Rendering.CullMode.Off);
                    lineMaterial.SetInt("_ZWrite", 0);
                }
            }
        }
        
        public void SetNavMeshData(NavMeshData data)
        {
            navMeshData = data;
            
            if (logNavMeshInfo && data != null)
            {
                RecastNavigationWrapper.LogNavMeshInfo(data);
            }
            
            UpdateVisualization();
        }
        
        public void SetVisualizationSettings(bool showNav, bool showWalkable, bool showUnwalkable, 
                                           Color navColor, Color walkColor, Color unwalkColor)
        {
            showNavMesh = showNav;
            showWalkableAreas = showWalkable;
            showUnwalkableAreas = showUnwalkable;
            navMeshColor = navColor;
            walkableColor = walkColor;
            unwalkableColor = unwalkColor;
            
            UpdateVisualization();
        }
        
        private void UpdateVisualization()
        {
            CleanupVisualization();
            
            if (navMeshData == null || !RecastNavigationWrapper.IsNavMeshValid(navMeshData))
            {
                return;
            }
            
            CreateVisualizationObjects();
        }
        
        private void CreateVisualizationObjects()
        {
            if (navMeshData.polygons == null) return;
            
            for (int i = 0; i < navMeshData.polygons.Length; i++)
            {
                var polygon = navMeshData.polygons[i];
                if (polygon.vertices == null || polygon.vertices.Length < 3) continue;
                
                // Create visualization object for this polygon
                GameObject polyObj = new GameObject($"NavMeshPolygon_{i}");
                polyObj.transform.SetParent(transform);
                polyObj.hideFlags = HideFlags.HideInHierarchy;
                
                // Add line renderer for polygon outline
                if (showNavMesh)
                {
                    LineRenderer lineRenderer = polyObj.AddComponent<LineRenderer>();
                    lineRenderer.material = lineMaterial;
                    lineRenderer.startWidth = lineWidth;
                    lineRenderer.endWidth = lineWidth;
                    lineRenderer.color = navMeshColor;
                    lineRenderer.useWorldSpace = true;
                    lineRenderer.loop = true;
                    
                    // Set vertices
                    Vector3[] vertices = new Vector3[polygon.vertices.Length + 1];
                    for (int j = 0; j < polygon.vertices.Length; j++)
                    {
                        vertices[j] = polygon.vertices[j];
                    }
                    vertices[polygon.vertices.Length] = polygon.vertices[0]; // Close the loop
                    
                    lineRenderer.positionCount = vertices.Length;
                    lineRenderer.SetPositions(vertices);
                }
                
                // Add mesh renderer for polygon fill
                if (showWalkableAreas || showUnwalkableAreas)
                {
                    Color fillColor = (polygon.area == 0) ? unwalkableColor : walkableColor;
                    bool shouldShow = (polygon.area == 0 && showUnwalkableAreas) || 
                                    (polygon.area != 0 && showWalkableAreas);
                    
                    if (shouldShow)
                    {
                        CreatePolygonMesh(polyObj, polygon.vertices, fillColor);
                    }
                }
                
                // Add center marker
                if (showPolygonCenters)
                {
                    CreateCenterMarker(polyObj, polygon.vertices);
                }
                
                visualizationObjects.Add(polyObj);
            }
        }
        
        private void CreatePolygonMesh(GameObject parent, Vector3[] vertices, Color color)
        {
            GameObject meshObj = new GameObject("PolygonMesh");
            meshObj.transform.SetParent(parent.transform);
            
            MeshFilter meshFilter = meshObj.AddComponent<MeshFilter>();
            MeshRenderer meshRenderer = meshObj.AddComponent<MeshRenderer>();
            
            // Create mesh
            Mesh mesh = new Mesh();
            mesh.vertices = vertices;
            
            // Create triangles (simple triangulation for convex polygons)
            int[] triangles = new int[(vertices.Length - 2) * 3];
            for (int i = 0; i < vertices.Length - 2; i++)
            {
                triangles[i * 3] = 0;
                triangles[i * 3 + 1] = i + 1;
                triangles[i * 3 + 2] = i + 2;
            }
            mesh.triangles = triangles;
            
            // Calculate normals
            mesh.RecalculateNormals();
            
            meshFilter.mesh = mesh;
            
            // Create material
            Material material = new Material(Shader.Find("Standard"));
            material.color = color;
            material.SetFloat("_Mode", 3); // Transparent mode
            material.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.SrcAlpha);
            material.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
            material.SetInt("_ZWrite", 0);
            material.DisableKeyword("_ALPHATEST_ON");
            material.EnableKeyword("_ALPHABLEND_ON");
            material.DisableKeyword("_ALPHAPREMULTIPLY_ON");
            material.renderQueue = 3000;
            
            meshRenderer.material = material;
        }
        
        private void CreateCenterMarker(GameObject parent, Vector3[] vertices)
        {
            GameObject markerObj = new GameObject("CenterMarker");
            markerObj.transform.SetParent(parent.transform);
            
            // Calculate center
            Vector3 center = Vector3.zero;
            for (int i = 0; i < vertices.Length; i++)
            {
                center += vertices[i];
            }
            center /= vertices.Length;
            
            markerObj.transform.position = center;
            
            // Create sphere
            GameObject sphere = GameObject.CreatePrimitive(PrimitiveType.Sphere);
            sphere.transform.SetParent(markerObj.transform);
            sphere.transform.localScale = Vector3.one * centerMarkerSize;
            
            // Set material
            MeshRenderer renderer = sphere.GetComponent<MeshRenderer>();
            Material material = new Material(Shader.Find("Standard"));
            material.color = Color.yellow;
            renderer.material = material;
        }
        
        private void CleanupVisualization()
        {
            foreach (var obj in visualizationObjects)
            {
                if (obj != null)
                {
                    if (Application.isPlaying)
                    {
                        DestroyImmediate(obj);
                    }
                    else
                    {
                        DestroyImmediate(obj);
                    }
                }
            }
            visualizationObjects.Clear();
        }
        
        // Public methods for editor interaction
        public void RefreshVisualization()
        {
            UpdateVisualization();
        }
        
        public void ClearVisualization()
        {
            CleanupVisualization();
        }
        
        public void ToggleDebugInfo()
        {
            showDebugInfo = !showDebugInfo;
        }
        
        public void TogglePolygonCenters()
        {
            showPolygonCenters = !showPolygonCenters;
            UpdateVisualization();
        }
        
        private void OnDrawGizmos()
        {
            if (navMeshData == null || !showDebugInfo) return;
            
            // Draw bounds
            Gizmos.color = Color.yellow;
            Gizmos.DrawWireCube(navMeshData.bounds.center, navMeshData.bounds.size);
        }
        
        private void OnValidate()
        {
            if (Application.isPlaying)
            {
                UpdateVisualization();
            }
        }
    }
} 
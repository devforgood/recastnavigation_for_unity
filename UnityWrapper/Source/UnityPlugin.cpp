#include <cstdio>
#include <cstring>
#include <vector>
#include <string>

// RecastNavigation includes
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "InputGeom.h"
#include "TestCase.h"
#include "Sample_SoloMesh.h"
#include "Sample.h"
#include "SampleInterfaces.h"
#include "MeshLoaderObj.h"

// Logging helper
#include "LogHelper.h"

// Unity plugin exports
#ifdef _WIN32
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API __attribute__((visibility("default")))
#endif

// DLL entry point for Windows
#ifdef _WIN32
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        // Initialize logging when DLL is loaded
        LogHelper::Initialize();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        // Cleanup logging when DLL is unloaded
        LogHelper::Cleanup();
        break;
    }
    return TRUE;
}
#else
// Constructor and destructor for Linux/macOS shared libraries
__attribute__((constructor))
void InitializeLibrary()
{
    LogHelper::Initialize();
}

__attribute__((destructor))
void CleanupLibrary()
{
    LogHelper::Cleanup();
}
#endif

extern "C" {

// NavMesh file format structures (from RecastDemo)
struct NavMeshSetHeader
{
    int magic;
    int version;
    int numTiles;
    dtNavMeshParams params;
};

struct NavMeshTileHeader
{
    dtTileRef tileRef;
    int dataSize;
};

// Magic numbers for navmesh file format
static const int NAVMESHSET_MAGIC = 'M'<<24 | 'S'<<16 | 'E'<<8 | 'T'; // 'MSET'
static const int NAVMESHSET_VERSION = 1;

// Global variables for managing navmesh data (only for generation)
static std::vector<dtNavMesh*> g_generatedNavMeshes;

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
    float maxEdgeLen,
    float detailSampleDistance,
    float detailSampleMaxError)
{
    try
    {
        LogHelper::LogPrintf("UnityWrapper Starting NavMesh generation from: %s\n", objFilePath);
        
        // Create build context
        rcContext ctx;
        
        // Load input geometry
        InputGeom geom;
        if (!geom.load(&ctx, objFilePath))
        {
            LogHelper::LogPrintf("Failed to load mesh: %s\n", objFilePath);
            return false;
        }

        LogHelper::LogPrintf("Mesh loaded successfully: %s\n", objFilePath);
        
        // Get mesh bounds and data
        const float* bmin = geom.getNavMeshBoundsMin();
        const float* bmax = geom.getNavMeshBoundsMax();
        const float* verts = geom.getMesh()->getVerts();
        const int nverts = geom.getMesh()->getVertCount();
        const int* tris = geom.getMesh()->getTris();
        const int ntris = geom.getMesh()->getTriCount();
        
        LogHelper::LogPrintf("Mesh bounds: [%.2f, %.2f, %.2f] to [%.2f, %.2f, %.2f]\n", 
               bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2]);
        LogHelper::LogPrintf("Vertices: %d, Triangles: %d\n", nverts, ntris);
        
        // Initialize build configuration
        rcConfig cfg;
        memset(&cfg, 0, sizeof(cfg));
        cfg.cs = cellSize;
        cfg.ch = cellHeight;
        cfg.walkableSlopeAngle = walkableSlopeAngle;
        cfg.walkableHeight = (int)ceilf(walkableHeight / cfg.ch);
        cfg.walkableClimb = (int)floorf(walkableClimb / cfg.ch);
        cfg.walkableRadius = (int)ceilf(walkableRadius / cfg.cs);
        cfg.maxEdgeLen = (int)(maxEdgeLen / cellSize);
        cfg.maxSimplificationError = maxSimplificationError;
        cfg.minRegionArea = (int)rcSqr(minRegionArea);
        cfg.mergeRegionArea = (int)rcSqr(mergeRegionArea);
        
        // Ensure minimum values for stability
        if (cfg.minRegionArea < 8) cfg.minRegionArea = 8;
        if (cfg.mergeRegionArea < 20) cfg.mergeRegionArea = 20;
        cfg.maxVertsPerPoly = 6;
        cfg.detailSampleDist = detailSampleDistance < 0.9f ? 0 : cellSize * detailSampleDistance;
        cfg.detailSampleMaxError = cellHeight * detailSampleMaxError;
        
        // Set the area where the navigation will be built
        rcVcopy(cfg.bmin, bmin);
        rcVcopy(cfg.bmax, bmax);
        rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);
        
        LogHelper::LogPrintf("Grid size: %d x %d\n", cfg.width, cfg.height);
        LogHelper::LogPrintf("Detailed params: cs=%.2f, ch=%.2f, walkableSlopeAngle=%.2f, maxEdgeLen=%d, maxSimplificationError=%.2f\n",
                            cfg.cs, cfg.ch, cfg.walkableSlopeAngle, cfg.maxEdgeLen, cfg.maxSimplificationError);
        
        // Reset build times
        ctx.resetTimers();
        ctx.startTimer(RC_TIMER_TOTAL);
        
        // Step 1. Rasterize input polygon soup
        LogHelper::LogPrintf("Step 1: Rasterizing input polygon soup...\n");
        rcHeightfield* solid = rcAllocHeightfield();
        if (!solid)
        {
            LogHelper::LogPrintf("Out of memory 'solid'.\n");
            return false;
        }
        if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
        {
            LogHelper::LogPrintf("Could not create solid heightfield.\n");
            rcFreeHeightField(solid);
            return false;
        }
        
        LogHelper::LogPrintf("Heightfield created: %d x %d cells\n", solid->width, solid->height);
        
        // Allocate array that can hold triangle area types
        unsigned char* triareas = new unsigned char[ntris];
        if (!triareas)
        {
            LogHelper::LogPrintf("Out of memory 'triareas'.\n");
            rcFreeHeightField(solid);
            return false;
        }
        
        // Find triangles which are walkable based on their slope and rasterize them
        memset(triareas, 0, ntris * sizeof(unsigned char));
        rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, verts, nverts, tris, ntris, triareas);
        if (!rcRasterizeTriangles(&ctx, verts, nverts, tris, triareas, ntris, *solid, cfg.walkableClimb))
        {
            LogHelper::LogPrintf("Could not rasterize triangles.\n");
            delete[] triareas;
            rcFreeHeightField(solid);
            return false;
        }
        
        delete[] triareas;
        
        // Step 2. Filter walkable surfaces
        LogHelper::LogPrintf("Step 2: Filtering walkable surfaces...\n");
        rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
        rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
        rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);
        
        // Step 3. Partition walkable surface to simple regions
        LogHelper::LogPrintf("Step 3: Partitioning walkable surface...\n");
        rcCompactHeightfield* chf = rcAllocCompactHeightfield();
        if (!chf)
        {
            LogHelper::LogPrintf("Out of memory 'chf'.\n");
            rcFreeHeightField(solid);
            return false;
        }
        if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf))
        {
            LogHelper::LogPrintf("Could not build compact heightfield.\n");
            rcFreeCompactHeightfield(chf);
            rcFreeHeightField(solid);
            return false;
        }
        
        LogHelper::LogPrintf("CompactHeightfield created: %d spans\n", chf->spanCount);
        
        rcFreeHeightField(solid);
        
        // Erode the walkable area by agent radius
        if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf))
        {
            LogHelper::LogPrintf("Could not erode walkable area.\n");
            rcFreeCompactHeightfield(chf);
            return false;
        }
        
        LogHelper::LogPrintf("After erosion: %d spans\n", chf->spanCount);
        
        // Mark areas
        const ConvexVolume* vols = geom.getConvexVolumes();
        for (int i = 0; i < geom.getConvexVolumeCount(); ++i)
        {
            rcMarkConvexPolyArea(&ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *chf);
        }
        
        // Step 3.5. Build regions
        LogHelper::LogPrintf("Step 3.5: Building regions...\n");
        LogHelper::LogPrintf("Region params: borderSize=%d, minRegionArea=%d, mergeRegionArea=%d\n", 
                            cfg.borderSize, cfg.minRegionArea, cfg.mergeRegionArea);
        
        // Use Watershed partitioning (same as RecastDemo default)
        LogHelper::LogPrintf("Using watershed partitioning...\n");
        
        // Prepare for region partitioning, by calculating distance field along the walkable surface.
        if (!rcBuildDistanceField(&ctx, *chf))
        {
            LogHelper::LogPrintf("Could not build distance field.\n");
            rcFreeCompactHeightfield(chf);
            return false;
        }
        
        // Partition the walkable surface into simple regions without holes.
        if (!rcBuildRegions(&ctx, *chf, 0, cfg.minRegionArea, cfg.mergeRegionArea))
        {
            LogHelper::LogPrintf("Could not build watershed regions.\n");
            rcFreeCompactHeightfield(chf);
            return false;
        }
        
        LogHelper::LogPrintf("Watershed regions built successfully\n");
        LogHelper::LogPrintf("Regions built successfully\n");
        
        // Step 4. Trace and simplify region contours
        LogHelper::LogPrintf("Step 4: Tracing and simplifying region contours...\n");
        rcContourSet* cset = rcAllocContourSet();
        if (!cset)
        {
            LogHelper::LogPrintf("Out of memory 'cset'.\n");
            rcFreeCompactHeightfield(chf);
            return false;
        }
        if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset))
        {
            LogHelper::LogPrintf("Could not build contours.\n");
            rcFreeContourSet(cset);
            rcFreeCompactHeightfield(chf);
            return false;
        }
        
        LogHelper::LogPrintf("ContourSet created: %d contours\n", cset->nconts);
        // ContourSet 생성 후 로그
        LogHelper::LogPrintf("ContourSet created: %d contours\n", cset->nconts);
        for (int i = 0; i < cset->nconts; ++i)
        {
            LogHelper::LogPrintf("  Contour %d: nverts=%d\n", i, cset->conts[i].nverts);
        }
        
        // Step 5. Build and triangulate contours
        LogHelper::LogPrintf("Step 5: Building and triangulating contours...\n");
        LogHelper::LogPrintf("Contour set info: %d contours\n", cset->nconts);
        
        rcPolyMesh* pmesh = rcAllocPolyMesh();
        if (!pmesh)
        {
            LogHelper::LogPrintf("Out of memory 'pmesh'.\n");
            rcFreeContourSet(cset);
            rcFreeCompactHeightfield(chf);
            return false;
        }
        if (!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh))
        {
            LogHelper::LogPrintf("Could not triangulate contours.\n");
            rcFreePolyMesh(pmesh);
            rcFreeContourSet(cset);
            rcFreeCompactHeightfield(chf);
            return false;
        }
        
        LogHelper::LogPrintf("PolyMesh created: %d vertices, %d polygons\n", pmesh->nverts, pmesh->npolys);
        // PolyMesh 생성 후 로그
        LogHelper::LogPrintf("PolyMesh created: %d vertices, %d polygons\n", pmesh->nverts, pmesh->npolys);
        
        // Step 6. Create detail mesh which allows to access approximate height on each polygon
        LogHelper::LogPrintf("Step 6: Creating detail mesh...\n");
        // PolyMesh, CompactHeightfield, DetailMesh 파라미터 로그
        LogHelper::LogPrintf("PolyMesh for Detail: nverts=%d, npolys=%d\n", pmesh->nverts, pmesh->npolys);
        LogHelper::LogPrintf("CompactHeightfield for Detail: spanCount=%d\n", chf->spanCount);
        LogHelper::LogPrintf("Detail params: sampleDist=%.2f, sampleMaxError=%.2f\n", cfg.detailSampleDist, cfg.detailSampleMaxError);
        rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
        if (!dmesh)
        {
            LogHelper::LogPrintf("Out of memory 'dmesh'.\n");
            rcFreePolyMesh(pmesh);
            rcFreeContourSet(cset);
            rcFreeCompactHeightfield(chf);
            return false;
        }
        if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh))
        {
            LogHelper::LogPrintf("Could not build polymesh detail.\n");
            rcFreePolyMeshDetail(dmesh);
            rcFreePolyMesh(pmesh);
            rcFreeContourSet(cset);
            rcFreeCompactHeightfield(chf);
            return false;
        }
        LogHelper::LogPrintf("DetailMesh: %d verts, %d tris\n", dmesh->nverts, dmesh->ntris);
        
        // Free intermediate data
        rcFreeCompactHeightfield(chf);
        rcFreeContourSet(cset);
        
        // Step 7. Create Detour data from Recast poly mesh
        LogHelper::LogPrintf("Step 7: Creating Detour data...\n");
        unsigned char* navData = 0;
        int navDataSize = 0;
        
        if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
        {
            LogHelper::LogPrintf("PolyMesh info: %d vertices, %d polygons, maxVertsPerPoly: %d\n", 
                                pmesh->nverts, pmesh->npolys, pmesh->nvp);
            
            // Update poly flags from areas
            for (int i = 0; i < pmesh->npolys; ++i)
            {
                if (pmesh->areas[i] == RC_WALKABLE_AREA)
                    pmesh->areas[i] = 1; // SAMPLE_POLYAREA_GROUND
                    
                if (pmesh->areas[i] == 1) // SAMPLE_POLYAREA_GROUND
                {
                    pmesh->flags[i] = 1; // SAMPLE_POLYFLAGS_WALK
                }
            }
            
            dtNavMeshCreateParams params;
            memset(&params, 0, sizeof(params));
            params.verts = pmesh->verts;
            params.vertCount = pmesh->nverts;
            params.polys = pmesh->polys;
            params.polyAreas = pmesh->areas;
            params.polyFlags = pmesh->flags;
            params.polyCount = pmesh->npolys;
            params.nvp = pmesh->nvp;
            params.detailMeshes = dmesh->meshes;
            params.detailVerts = dmesh->verts;
            params.detailVertsCount = dmesh->nverts;
            params.detailTris = dmesh->tris;
            params.detailTriCount = dmesh->ntris;
            params.offMeshConVerts = geom.getOffMeshConnectionVerts();
            params.offMeshConRad = geom.getOffMeshConnectionRads();
            params.offMeshConDir = geom.getOffMeshConnectionDirs();
            params.offMeshConAreas = geom.getOffMeshConnectionAreas();
            params.offMeshConFlags = geom.getOffMeshConnectionFlags();
            params.offMeshConUserID = geom.getOffMeshConnectionId();
            params.offMeshConCount = geom.getOffMeshConnectionCount();
            params.walkableHeight = walkableHeight;
            params.walkableRadius = walkableRadius;
            params.walkableClimb = walkableClimb;
            rcVcopy(params.bmin, pmesh->bmin);
            rcVcopy(params.bmax, pmesh->bmax);
            params.cs = cfg.cs;
            params.ch = cfg.ch;
            params.buildBvTree = true;
            
            LogHelper::LogPrintf("Detour params: vertCount=%d, polyCount=%d, nvp=%d, detailVertsCount=%d, detailTriCount=%d\n",
                                params.vertCount, params.polyCount, params.nvp, params.detailVertsCount, params.detailTriCount);
            LogHelper::LogPrintf("Agent params: height=%.2f, radius=%.2f, climb=%.2f\n",
                                params.walkableHeight, params.walkableRadius, params.walkableClimb);
            
            // Validate parameters before calling dtCreateNavMeshData
            if (params.vertCount >= 0xffff)
            {
                LogHelper::LogPrintf("Error: Too many vertices (%d >= 65535)\n", params.vertCount);
                rcFreePolyMeshDetail(dmesh);
                rcFreePolyMesh(pmesh);
                return false;
            }
            
            if (params.nvp > DT_VERTS_PER_POLYGON)
            {
                LogHelper::LogPrintf("Error: Too many vertices per polygon (%d > %d)\n", params.nvp, DT_VERTS_PER_POLYGON);
                rcFreePolyMeshDetail(dmesh);
                rcFreePolyMesh(pmesh);
                return false;
            }
            
            if (!params.vertCount || !params.verts)
            {
                LogHelper::LogPrintf("Error: Invalid vertex data\n");
                rcFreePolyMeshDetail(dmesh);
                rcFreePolyMesh(pmesh);
                return false;
            }
            
            if (!params.polyCount || !params.polys)
            {
                LogHelper::LogPrintf("Error: Invalid polygon data\n");
                rcFreePolyMeshDetail(dmesh);
                rcFreePolyMesh(pmesh);
                return false;
            }
            
            if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
            {
                LogHelper::LogPrintf("Could not build Detour navmesh.\n");
                LogHelper::LogPrintf("Detour navmesh creation FAILED\n");
                rcFreePolyMeshDetail(dmesh);
                rcFreePolyMesh(pmesh);
                return false;
            }
            
            LogHelper::LogPrintf("Detour navmesh creation SUCCESS\n");
        }
        
        // Free intermediate data
        rcFreePolyMeshDetail(dmesh);
        rcFreePolyMesh(pmesh);
        
        // Step 8. Save navmesh data to file
        LogHelper::LogPrintf("Step 8: Saving navmesh data to file...\n");
        if (navData && navDataSize > 0)
        {
            // Create a temporary navmesh to get the proper file format
            dtNavMesh* tempNavMesh = dtAllocNavMesh();
            if (!tempNavMesh)
            {
                LogHelper::LogPrintf("Failed to allocate temporary navmesh for saving\n");
                dtFree(navData);
                return false;
            }
            
            // Initialize the temporary navmesh with the generated data
            dtStatus status = tempNavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
            if (dtStatusFailed(status))
            {
                LogHelper::LogPrintf("Failed to initialize temporary navmesh for saving. Status: %d\n", status);
                dtFreeNavMesh(tempNavMesh);
                dtFree(navData);
                return false;
            }
            
            // Save using the proper file format (like RecastDemo's saveAll)
            FILE* file = fopen(outputPath, "wb");
            if (file)
            {
                // Store header
                NavMeshSetHeader header;
                header.magic = NAVMESHSET_MAGIC;
                header.version = NAVMESHSET_VERSION;
                header.numTiles = 0;
                
                // Count tiles
                for (int i = 0; i < tempNavMesh->getMaxTiles(); ++i)
                {
                    const dtMeshTile* tile = ((const dtNavMesh * )tempNavMesh)->getTile(i);
                    if (!tile || !tile->header || !tile->dataSize) continue;
                    header.numTiles++;
                }
                
                memcpy(&header.params, tempNavMesh->getParams(), sizeof(dtNavMeshParams));
                fwrite(&header, sizeof(NavMeshSetHeader), 1, file);
                
                // Store tiles
                for (int i = 0; i < tempNavMesh->getMaxTiles(); ++i)
                {
                    const dtMeshTile* tile = ((const dtNavMesh*)tempNavMesh)->getTile(i);
                    if (!tile || !tile->header || !tile->dataSize) continue;
                    
                    NavMeshTileHeader tileHeader;
                    tileHeader.tileRef = tempNavMesh->getTileRef(tile);
                    tileHeader.dataSize = tile->dataSize;
                    fwrite(&tileHeader, sizeof(tileHeader), 1, file);
                    
                    fwrite(tile->data, tile->dataSize, 1, file);
                }
                
                fclose(file);
                
                LogHelper::LogPrintf("NavMesh data written successfully with proper header: %s (%d tiles)\n", outputPath, header.numTiles);
                
                ctx.stopTimer(RC_TIMER_TOTAL);
                LogHelper::LogPrintf("Total build time: %.2f ms\n", ctx.getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f);
                
                dtFreeNavMesh(tempNavMesh);
                return true;
            }
            else
            {
                LogHelper::LogPrintf("Could not open output file for writing: %s\n", outputPath);
                dtFreeNavMesh(tempNavMesh);
                dtFree(navData);
                return false;
            }
        }
        else
        {
            LogHelper::LogPrintf("No navmesh data generated.\n");
            return false;
        }
    }
    catch (const std::exception& e)
    {
        LogHelper::LogPrintf("Exception during navmesh generation: %s\n", e.what());
        return false;
    }
}

// Cleanup all generated navmeshes
EXPORT_API void CleanupAllNavMeshData()
{
    for (auto* navMesh : g_generatedNavMeshes)
    {
        dtFreeNavMesh(navMesh);
    }
    g_generatedNavMeshes.clear();
}

} // extern "C" 
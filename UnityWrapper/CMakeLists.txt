cmake_minimum_required(VERSION 3.16)
project(RecastNavigationUnity)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)

# Include directories
include_directories(../DebugUtils/Include)
include_directories(../Detour/Include)
include_directories(../DetourCrowd/Include)
include_directories(../DetourTileCache/Include)
include_directories(../Recast/Include)
include_directories(../RecastDemo/Include)
include_directories(../RecastDemo/Contrib/fastlz)
include_directories(../RecastDemo/Contrib)

# Source files for Unity plugin (simplified version)
set(UNITY_PLUGIN_SOURCES
    Source/UnityPlugin.cpp
    Source/LogHelper.cpp
    Source/MeshLoaderObj.cpp
    Source/InputGeom.cpp
    Source/ChunkyTriMesh.cpp
    Source/ValueHistory.cpp
    ../RecastDemo/Contrib/fastlz/fastlz.c
)

# Create Unity plugin library
if(WIN32)
    add_library(RecastNavigationUnity SHARED ${UNITY_PLUGIN_SOURCES})
    set_target_properties(RecastNavigationUnity PROPERTIES
        OUTPUT_NAME "RecastNavigationUnity"
        PREFIX ""
        SUFFIX ".dll"
    )
elseif(APPLE)
    add_library(RecastNavigationUnity SHARED ${UNITY_PLUGIN_SOURCES})
    set_target_properties(RecastNavigationUnity PROPERTIES
        OUTPUT_NAME "RecastNavigationUnity"
        PREFIX ""
        SUFFIX ".bundle"
    )
else()
    add_library(RecastNavigationUnity SHARED ${UNITY_PLUGIN_SOURCES})
    set_target_properties(RecastNavigationUnity PROPERTIES
        OUTPUT_NAME "RecastNavigationUnity"
        PREFIX "lib"
        SUFFIX ".so"
    )
endif()

# Link libraries
target_link_libraries(RecastNavigationUnity
    DebugUtils
    Detour
    DetourCrowd
    DetourTileCache
    Recast
    ${OPENGL_LIBRARIES}
)

# Set output directory for Unity plugin
set_target_properties(RecastNavigationUnity PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Runtime/Plugins/x86_64"
)

# Post-build: Copy DLL to UnityDemo/Assets/Plugins/Editor
if(WIN32)
    add_custom_command(TARGET RecastNavigationUnity POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/UnityDemo/Assets/Plugins/Editor"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:RecastNavigationUnity>"
            "${CMAKE_SOURCE_DIR}/UnityDemo/Assets/Plugins/Editor/"
    )
endif()

# Install target
install(TARGETS RecastNavigationUnity
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
) 
# CMake version
cmake_minimum_required(VERSION 3.19)

# vcpkg auto-download and integration
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/vcpkg")
    # Download and install vcpkg if not found
    include(FetchContent)
    
    # Set vcpkg installation location 
    set(VCPKG_INSTALL_DIR "${CMAKE_SOURCE_DIR}/vcpkg" CACHE PATH "Where to install vcpkg")
    message(STATUS "VCPKG install dir: ${VCPKG_INSTALL_DIR}")
    
    # Download vcpkg
    FetchContent_Declare(
        vcpkg
        GIT_REPOSITORY https://github.com/Microsoft/vcpkg.git
        GIT_TAG master
        SOURCE_DIR "${VCPKG_INSTALL_DIR}"
    )
    
    FetchContent_GetProperties(vcpkg)
    if(NOT vcpkg_POPULATED)
        message(STATUS "Downloading vcpkg...")
        FetchContent_Populate(vcpkg)
        
        # Bootstrap vcpkg
        if(UNIX)
            message(STATUS "Bootstrapping vcpkg...")
            execute_process(
                COMMAND ./bootstrap-vcpkg.sh
                WORKING_DIRECTORY "${VCPKG_INSTALL_DIR}"
                RESULT_VARIABLE bootstrap_result
            )
        else()
            message(STATUS "Bootstrapping vcpkg...")
            execute_process(
                COMMAND ./bootstrap-vcpkg.bat
                WORKING_DIRECTORY "${VCPKG_INSTALL_DIR}"
                RESULT_VARIABLE bootstrap_result
            )
        endif()
        
        if(NOT bootstrap_result EQUAL 0)
            message(FATAL_ERROR "Failed to bootstrap vcpkg")
        endif()
    endif()
    
    set(VCPKG_ROOT "${VCPKG_INSTALL_DIR}" CACHE PATH "vcpkg root directory" FORCE)
endif()

# Set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
CACHE STRING "Vcpkg toolchain file")

file(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" VCPKG_JSON)
string(JSON PROJECT_NAME GET "${VCPKG_JSON}" "name")
string(JSON PROJECT_VERSION GET "${VCPKG_JSON}" "version")

project(${PROJECT_NAME} VERSION ${PROJECT_VERSION})

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)

option(BUILD_EXAMPLES "Build the example programs" ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Building ${PROJECT_NAME} in Debug mode")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Building ${PROJECT_NAME} in Release mode")
endif()

# External libraries
# Bullet - physics library
find_package(Bullet CONFIG REQUIRED)

# GLFW, stb, tinyfiledialogs, iniparser
find_package(glfw3 CONFIG REQUIRED)
find_package(tinyfiledialogs CONFIG REQUIRED)
find_package(unofficial-iniparser CONFIG REQUIRED)

# OpenSceneGraph + OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
set(osg_OPENGL_PROFILE GL3)
find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgViewer osgDB osgGA osgUtil)

find_package(assimp CONFIG REQUIRED)

# ImGui
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/ImGuiInclude)

# EventHandler
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/EventHandler)

# STB_Image
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/extern/stb_image)

# Set config file with variables 
set(RESOURCES_PATH "${CMAKE_CURRENT_SOURCE_DIR}/resources")
# set(FONT_NAME "Monocraft.otf")
set(FONT_NAME "Cruinn_Bold.ttf")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_BUILD_STRING "DEBUG")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_BUILD_STRING "RELEASE")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/CMake/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/include/config.h @ONLY)

# Add include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Add the source files for your project
# Set 'GUI_Source' variable 
file(GLOB_RECURSE GUI_Sources CONFIGURE_DEPENDS "include/*.h"
                                                "src/*.cpp")
                                                
# Build GULAG library
if(${BUILD_SHARED_LIBS})
    add_library(${PROJECT_NAME} SHARED ${GUI_Sources})
else()
    add_library(${PROJECT_NAME} STATIC ${GUI_Sources})
endif()

# target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Debug>:/MTd>)
# target_compile_options(${PROJECT_NAME} PUBLIC $<$<CONFIG:Release>:/MT>)

# Link and include external libraries
# target_link_libraries(${PROJECT_NAME} PUBLIC Eigen3::Eigen)

target_link_libraries(${PROJECT_NAME} PUBLIC ${BULLET_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${BULLET_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC stb_image)
target_link_libraries(${PROJECT_NAME} PUBLIC tinyfiledialogs::tinyfiledialogs)
target_link_libraries(${PROJECT_NAME} PUBLIC unofficial::iniparser::iniparser)

target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENSCENEGRAPH_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENSCENEGRAPH_INCLUDE_DIRS})
# Link OpenSceneGraph plugins only in Release
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Linking OpenSceneGraph plugins")
    if(UNIX) 
        file(GLOB OSGDB_LIB_FILES "${VCPKG_INSTALLED_DIR}/x64-linux/lib/osgPlugins-3.6.5/*.a")
    else()
        file(GLOB OSGDB_LIB_FILES "${VCPKG_INSTALLED_DIR}/x64-windows-static/lib/osgPlugins-3.6.5/*.lib")
    endif()
    target_link_libraries(${PROJECT_NAME} PUBLIC ${OSGDB_LIB_FILES})
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC assimp::assimp)

# Link ImGui
target_link_libraries(${PROJECT_NAME} PUBLIC imguiInclude)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern/ImGuiInclude)

# Link EventHandler
target_link_libraries(${PROJECT_NAME} PUBLIC EventHandler)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/extern/EventHandler/include)

# Example
if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

# Copy resources to bin

# Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/resources
                   ${CMAKE_SOURCE_DIR}/bin/Debug/resources
                   
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/CMake/imgui.ini
                   ${CMAKE_SOURCE_DIR}/bin/Debug/imgui.ini)
endif()

# Release
if (CMAKE_BUILD_TYPE STREQUAL "Release")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                   ${CMAKE_CURRENT_SOURCE_DIR}/resources
                   ${CMAKE_SOURCE_DIR}/bin/Release/resources
                   
                   COMMAND ${CMAKE_COMMAND} -E copy
                   ${CMAKE_CURRENT_SOURCE_DIR}/CMake/imgui.ini
                   ${CMAKE_SOURCE_DIR}/bin/Release/imgui.ini)
endif()
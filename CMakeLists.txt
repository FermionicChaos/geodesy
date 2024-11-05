# Figure out how to package libraries together in geodesy.
cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 NEW)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    message(STATUS "Detected MSVC C++ Compiler Set")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
else()
    message(STATUS "Detected GNU C++ Compiler Set")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# ----------------------- Dependencies ----------------------- #

include(FetchContent)

# Setup directories for dependencies
#set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/dep")

# ----- Assimp ----- #
FetchContent_Declare(
    assimp
    GIT_REPOSITORY      https://github.com/assimp/assimp.git
    GIT_TAG             v5.4.3
)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Disable Assimp tests")
# set(ASSIMP_USE_STATIC_CRT ON CACHE BOOL "Use static CRT")
FetchContent_MakeAvailable(assimp)

# ----- Freetype ----- #
FetchContent_Declare(
    freetype
    GIT_REPOSITORY      https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG             VER-2-12-1
)
FetchContent_MakeAvailable(freetype)
file(READ ${FETCHCONTENT_BASE_DIR}/freetype-build/include/freetype/config/ftoption.h FreetypeConfig)
string(REGEX REPLACE "#define FT_CONFIG_OPTION_USE_ZLIB" "// #define FT_CONFIG_OPTION_USE_ZLIB" FreetypeConfig "${FreetypeConfig}")
file(WRITE ${FETCHCONTENT_BASE_DIR}/freetype-build/include/freetype/config/ftoption.h "${FreetypeConfig}")

# ----- STB ----- #
FetchContent_Declare(
    stb
    GIT_REPOSITORY      https://github.com/nothings/stb.git
    GIT_TAG             master
)
FetchContent_MakeAvailable(stb)

# ----- GLFW ----- #
FetchContent_Declare(
    glfw
    GIT_REPOSITORY      https://github.com/glfw/glfw.git
    GIT_TAG             3.3.8
)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_VULKAN_STATIC ON)
FetchContent_MakeAvailable(glfw)

# ----- PortAudio ----- #
# FetchContent_Declare(
#     portaudio
#     GIT_REPOSITORY      https://github.com/PortAudio/portaudio.git
#     GIT_TAG             v19.7.0
# )
# set(PA_BUILD_SHARED OFF)
# if (CMAKE_SYSTEM_NAME MATCHES "Windows")
#     set(PA_DLL_LINK_WITH_STATIC_RUNTIME OFF)
# endif()
# FetchContent_MakeAvailable(portaudio)

# ----- GLSLang ----- #
FetchContent_Declare(
    glslang
    GIT_REPOSITORY      https://github.com/KhronosGroup/glslang.git
    GIT_TAG             vulkan-sdk-1.3.296.0 # Find a way to the version that matches installed vulkan sdk
)
set(ENABLE_PCH OFF)
set(GLSLANG_ENABLE_INSTALL OFF)
set(GLSLANG_TESTS OFF)
set(ENABLE_OPT OFF)
FetchContent_MakeAvailable(glslang)

# ----- ImGui ----- #
FetchContent_Declare(
    imgui
    GIT_REPOSITORY      https://github.com/ocornut/imgui.git
    GIT_TAG             v1.90.4
)
FetchContent_MakeAvailable(imgui)
set(IMGUI_SOURCES
    ${FETCHCONTENT_BASE_DIR}/imgui-src/imgui.cpp
    ${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_draw.cpp
    ${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_tables.cpp
    ${FETCHCONTENT_BASE_DIR}/imgui-src/imgui_widgets.cpp
    ${FETCHCONTENT_BASE_DIR}/imgui-src/backends/imgui_impl_glfw.cpp
    ${FETCHCONTENT_BASE_DIR}/imgui-src/backends/imgui_impl_opengl3.cpp
)

# ----------------------- Geodesy Library ----------------------- #

set(GEODESY_LIBRARY "geodesy")
# set(GEODESY_UNIT_TEST "geodesy-unit-test")

file(GLOB_RECURSE INC
    "inc/*.h"
)

file(GLOB_RECURSE SRC
    "src/*.h"
    "src/*.cpp"
    "src/*.c"
)

project(${GEODESY_LIBRARY})

# Find and include OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message("OpenMP found.")
endif()

# Find and include Vulkan
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    message("Vulkan found.")
    # Find the last occurrence of "/Include" in the path
    string(FIND "${Vulkan_INCLUDE_DIRS}" "/Include" REMOVE_INDEX REVERSE)

    if (REMOVE_INDEX GREATER -1)
        # Remove the "/Include" part from the path
        string(SUBSTRING "${Vulkan_INCLUDE_DIRS}" 0 ${REMOVE_INDEX} VULKAN_SDK_ROOT_DIR)
    else ()
        # If "/Include" is not found, keep the original path
        set(VULKAN_SDK_ROOT_DIR "${Vulkan_INCLUDE_DIRS}")
    endif ()

    # Print the modified path
    message(STATUS "Vulkan SDK Root Path: ${VULKAN_SDK_ROOT_DIR}")
endif()

# Link against Vulkan and OpenMP
add_library(${GEODESY_LIBRARY} ${INC} ${SRC} ${IMGUI_SOURCES})

# Removed so it can be linked against other projects.
# set_target_properties(${GEODESY_LIBRARY} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}/)

# Include directories
target_include_directories(${GEODESY_LIBRARY} PRIVATE ${Vulkan_INCLUDE_DIRS}/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${CMAKE_SOURCE_DIR}/inc/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/glfw-src/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/glfw-build/src/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/glslang-src/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/stb-src/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/assimp-src/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/freetype-src/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/imgui-src/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/freetype-build/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${FETCHCONTENT_BASE_DIR}/assimp-build/include/)

# Link Against Vulkan
if(OpenMP_CXX_FOUND)
    target_link_libraries(${GEODESY_LIBRARY} PRIVATE OpenMP::OpenMP_CXX)
endif()

# Link Against Vulkan
if(Vulkan_FOUND)
    target_link_libraries(${GEODESY_LIBRARY} PRIVATE ${Vulkan_LIBRARY})
endif()

# Link Third Party Libraries
target_link_libraries(${GEODESY_LIBRARY} PUBLIC glfw)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC glslang)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC SPIRV)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC assimp)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC freetype)

# # ----------------------- Geodesy Unit Test ----------------------- #
# # When creating your own project off of the Geodesy Library, use this
# # as a template for your own CMakeLists.txt file to know what to link against.

# project(${GEODESY_UNIT_TEST})

# # Find and include OpenMP
# find_package(OpenMP REQUIRED)
# if(OpenMP_CXX_FOUND)
#     message("OpenMP found.")
# endif()

# # Find and include Vulkan
# find_package(Vulkan REQUIRED)
# if(Vulkan_FOUND)
#     message("Vulkan found.")
# endif()

# # Link against Vulkan and OpenMP
# add_executable(${GEODESY_UNIT_TEST} main.cpp res/icon.rc)

# set_target_properties(${GEODESY_UNIT_TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}/)

# # Include directories
# target_include_directories(${GEODESY_UNIT_TEST} PRIVATE ${Vulkan_INCLUDE_DIRS}/)
# target_include_directories(${GEODESY_UNIT_TEST} PRIVATE ${CMAKE_SOURCE_DIR}/inc/)

# # Link Against Vulkan
# if(OpenMP_CXX_FOUND)
#     target_link_libraries(${GEODESY_UNIT_TEST} PRIVATE OpenMP::OpenMP_CXX)
# endif()

# # Link Against Vulkan
# if(Vulkan_FOUND)
#     target_link_libraries(${GEODESY_UNIT_TEST} PRIVATE ${Vulkan_LIBRARY})
# endif()

# # Link Against Geodesy Library
# target_link_libraries(${GEODESY_UNIT_TEST} PRIVATE ${GEODESY_LIBRARY})

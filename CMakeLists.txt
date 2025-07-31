# Geodesy Engine - Cross-Platform Game Engine
cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0077 NEW)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ======================= BUILD OPTIONS =======================

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer in debug builds" OFF)
option(ENABLE_PROFILING "Enable profiling support" OFF)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)

# =======================================================================
#  GEODESY – COMPILER & BUILD-TYPE DETECTION
# =======================================================================

# Initialize compiler variables
set(GEODESY_COMPILER_CLANG  FALSE)
set(GEODESY_COMPILER_GCC    FALSE)
set(GEODESY_COMPILER_MSVC   FALSE)
set(GEODESY_COMPILER_INTEL  FALSE)
set(GEODESY_COMPILER_NAME   "Unknown")

# ---------- compiler family -------------------------------------------
# CMAKE_CXX_COMPILER_ID is always defined and the most reliable test.
# Additional checks (CMAKE_CXX_SIMULATE_ID) catch clang-cl.
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(GEODESY_COMPILER_MSVC TRUE)
    set(GEODESY_COMPILER_NAME "MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(GEODESY_COMPILER_GCC  TRUE)
    set(GEODESY_COMPILER_NAME "GCC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(Apple)?Clang$")
    set(GEODESY_COMPILER_CLANG TRUE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
        set(GEODESY_COMPILER_NAME "Apple Clang")
    elseif(MSVC AND CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")   # clang-cl
        set(GEODESY_COMPILER_NAME "Clang-CL")
    else()
        set(GEODESY_COMPILER_NAME "Clang")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "^(Intel|IntelLLVM)$")
    # oneAPI ICX/ICC front-ends are LLVM based
    set(GEODESY_COMPILER_INTEL TRUE)
    set(GEODESY_COMPILER_NAME "Intel LLVM")
elseif (CMAKE_GENERATOR MATCHES "Visual Studio")
    # Infer compiler from CMAKE_GENERATOR
    set(GEODESY_COMPILER_MSVC TRUE)
    set(GEODESY_COMPILER_NAME "MSVC")
else()
    message(WARNING "Geodesy: unknown compiler family: ${CMAKE_CXX_COMPILER_ID}")
    set(GEODESY_COMPILER_NAME "Unknown")
endif()

# ======================================================================
#  BUILD-TYPE FLAGS
#  • Single-config generators   → evaluate once.
#  • Multi-config generators    → provide generator expressions.
# ======================================================================

set(GEODESY_BUILD_DEBUG   FALSE)
set(GEODESY_BUILD_RELEASE FALSE)
set(GEODESY_BUILD_MODE    "Unknown")

if(NOT CMAKE_CONFIGURATION_TYPES)           # ---- Make / Ninja, etc.
    if(CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug$")
        set(GEODESY_BUILD_DEBUG TRUE)
        set(GEODESY_BUILD_MODE "Debug")
    else()                                   # Release / RelWithDebInfo / MinSizeRel / empty
        set(GEODESY_BUILD_RELEASE TRUE)
        set(GEODESY_BUILD_MODE "Release")
    endif()
else()                                       # ---- VS / Xcode
    # Export generator-expression helpers so you can write:
    #   target_compile_options(foo PRIVATE $<$<BOOL:${GEODESY_CLANG}>:-Wall>)
    set(GEODESY_BUILD_DEBUG "$<CONFIG:Debug>")
    set(GEODESY_BUILD_RELEASE "$<CONFIG:Release;RelWithDebInfo;MinSizeRel>")
    set(GEODESY_BUILD_MODE "Multi-Configuration")
endif()

# ======================= SYSTEM & PLATFORM DETECTION =======================

# Initialize system variables
set(GEODESY_SYSTEM_WINDOWS FALSE)
set(GEODESY_SYSTEM_LINUX FALSE)
set(GEODESY_SYSTEM_MACOS FALSE)
set(GEODESY_SYSTEM_IOS FALSE)
set(GEODESY_SYSTEM_ANDROID FALSE)
set(GEODESY_SYSTEM_NAME "Unknown")

# Initialize platform variables
set(GEODESY_DESKTOP FALSE)
set(GEODESY_MOBILE FALSE)
set(GEODESY_PLATFORM_NAME "Unknown")

# Detect target platform
if(WIN32)
    set(GEODESY_SYSTEM_WINDOWS TRUE)
    set(GEODESY_DESKTOP TRUE)
    set(GEODESY_SYSTEM_NAME "Windows")
    set(GEODESY_PLATFORM_NAME "Desktop")
elseif(UNIX AND NOT APPLE)
    if(ANDROID)
        set(GEODESY_SYSTEM_ANDROID TRUE)
        set(GEODESY_MOBILE TRUE)
        set(GEODESY_SYSTEM_NAME "Android")
        set(GEODESY_PLATFORM_NAME "Mobile")
    else()
        set(GEODESY_SYSTEM_LINUX TRUE)
        set(GEODESY_DESKTOP TRUE)
        set(GEODESY_SYSTEM_NAME "Linux")
        set(GEODESY_PLATFORM_NAME "Desktop")
    endif()
elseif(APPLE)
    if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(GEODESY_SYSTEM_IOS TRUE)
        set(GEODESY_MOBILE TRUE)
        set(GEODESY_SYSTEM_NAME "iOS")
        set(GEODESY_PLATFORM_NAME "Mobile")
    else()
        set(GEODESY_SYSTEM_MACOS TRUE)
        set(GEODESY_DESKTOP TRUE)
        set(GEODESY_SYSTEM_NAME "macOS")
        set(GEODESY_PLATFORM_NAME "Desktop")
    endif()
endif()

# ======================= COMPILER FLAGS =======================

# Set compiler-specific flags
if(GEODESY_COMPILER_MSVC)
    # MSVC-specific flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-")
    
    if(GEODESY_DEBUG)
        # Debug build: Maximum debug info, no optimization, runtime checks
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")           # High warning level
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Od")           # Disable optimization
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")           # Full debug info
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /RTC1")         # Runtime checks
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")          # Debug runtime library
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DDEBUG")       # Debug macro
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D_DEBUG")      # Debug macro
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")       # Large object files
    else()
        # Release build: Maximum speed optimization, minimal debug info
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /W1")       # Minimal warnings
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")       # Maximum speed optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi")       # Enable intrinsics
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Ot")       # Favor speed over size
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oy")       # Omit frame pointers
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")       # Whole program optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG")  # Release macro
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")       # Release runtime library
        # Link-time optimizations
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
        set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS_RELEASE} /LTCG")
    endif()
    
elseif(GEODESY_COMPILER_GCC OR GEODESY_COMPILER_CLANG)
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    
    if(GEODESY_DEBUG)
        # Debug build: Maximum debug info, no optimization, runtime checks
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")         # Enable warnings
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")       # Extra warnings
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wpedantic")    # Pedantic warnings
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")           # No optimization
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g3")           # Maximum debug info
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb")         # GDB-specific debug info
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer") # Keep frame pointers
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-optimize-sibling-calls") # Better stack traces
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")       # Debug macro
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")      # Debug macro
        
        # Additional GCC-specific debug flags
        if(GEODESY_COMPILER_GCC)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fvar-tracking-assignments") # Better variable tracking
        endif()
        
        # Sanitizers for debug builds (optional, can be enabled with -DENABLE_SANITIZERS=ON)
        if(ENABLE_SANITIZERS)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=undefined")
            message(STATUS "Sanitizers enabled for debug build")
        endif()
        
        # Profiling support
        if(ENABLE_PROFILING)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -pg")
            message(STATUS "Profiling enabled for debug build")
        endif()
        
        # Code coverage
        if(ENABLE_COVERAGE)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --coverage")
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} --coverage")
            message(STATUS "Code coverage enabled for debug build")
        endif()
        
    else()
        # Release build: Maximum speed optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")           # Maximum optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")      # Release macro
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -ffast-math")   # Fast math operations
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -funroll-loops") # Loop unrolling
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer") # Omit frame pointers
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native") # Target native architecture
        
        # Link-time optimization
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
        
        # Additional optimizations for specific compilers
        if(GEODESY_COMPILER_GCC)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fuse-linker-plugin") # Better LTO
        endif()
        
        if(GEODESY_COMPILER_CLANG)
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fvectorize")     # Auto-vectorization
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fslp-vectorize") # SLP vectorization
        endif()
    endif()
    
    # Platform-specific flags
    if(GEODESY_SYSTEM_ANDROID)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    elseif(GEODESY_SYSTEM_IOS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode")
        # iOS release builds should be more conservative with optimizations
        if(GEODESY_RELEASE)
            # Remove -march=native for iOS (not applicable)
            string(REPLACE "-march=native" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
        endif()
    endif()
endif()

# ======================= PRINT FULL SUMMARY =======================
message(STATUS "========================= Geodesy Build Summary =========================")
message(STATUS "Generator: \t${CMAKE_GENERATOR}")
message(STATUS "Compiler: \t${GEODESY_COMPILER_NAME} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "System: \t${GEODESY_SYSTEM_NAME}")
message(STATUS "Platform: \t${GEODESY_PLATFORM_NAME}")
message(STATUS "Build Mode: \t${GEODESY_BUILD_MODE}")
message(STATUS "========================= Geodesy Build Summary =========================")

# ======================= DEPENDENCIES =======================

include(FetchContent)

# Setup source directories for dependencies (build dirs will remain in build folder)
set(DEP_SOURCE_DIR "${CMAKE_SOURCE_DIR}/dep")

# ----- Assimp ----- #
FetchContent_Declare(
    assimp
    GIT_REPOSITORY      https://github.com/assimp/assimp.git
    GIT_TAG             v5.4.3
    SOURCE_DIR          ${DEP_SOURCE_DIR}/assimp-src
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
    SOURCE_DIR          ${DEP_SOURCE_DIR}/freetype-src
)
FetchContent_MakeAvailable(freetype)
file(READ ${freetype_BINARY_DIR}/include/freetype/config/ftoption.h FreetypeConfig)
string(REGEX REPLACE "#define FT_CONFIG_OPTION_USE_ZLIB" "// #define FT_CONFIG_OPTION_USE_ZLIB" FreetypeConfig "${FreetypeConfig}")
file(WRITE ${freetype_BINARY_DIR}/include/freetype/config/ftoption.h "${FreetypeConfig}")

# ----- STB ----- #
FetchContent_Declare(
    stb
    GIT_REPOSITORY      https://github.com/nothings/stb.git
    GIT_TAG             master
    SOURCE_DIR          ${DEP_SOURCE_DIR}/stb-src
)
FetchContent_MakeAvailable(stb)

if (GEODESY_DESKTOP)
    # ----- GLFW ----- #
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY      https://github.com/glfw/glfw.git
        GIT_TAG             3.3.8
        SOURCE_DIR          ${DEP_SOURCE_DIR}/glfw-src
    )
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_VULKAN_STATIC ON)
    FetchContent_MakeAvailable(glfw)
endif()

# ----- OpenXR ----- #
FetchContent_Declare(
    openxr
    GIT_REPOSITORY      https://github.com/KhronosGroup/OpenXR-SDK.git
    GIT_TAG             release-1.1.43
    SOURCE_DIR          ${DEP_SOURCE_DIR}/openxr-src
)
# set(BUILD_API_LAYERS OFF)
# set(BUILD_API_TESTS OFF)
# set(BUILD_LOADER ON)
# set(BUILD_TESTS OFF)
# set(BUILD_WITH_INSTALL_NAME ON)
# set(INSTALL_LOADER ON)
# set(INSTALL_TESTS OFF)
# set(USE_LOADER OFF)
FetchContent_MakeAvailable(openxr)

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
    GIT_TAG             vulkan-sdk-1.3.296.0
    SOURCE_DIR          ${DEP_SOURCE_DIR}/glslang-src
)
set(ENABLE_PCH OFF)
set(GLSLANG_ENABLE_INSTALL OFF)
set(GLSLANG_TESTS OFF)
set(ENABLE_OPT OFF)
FetchContent_MakeAvailable(glslang)

if (GEODESY_DESKTOP)
    # ----- ImGui ----- #
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY      https://github.com/ocornut/imgui.git
        GIT_TAG             v1.90.4
        SOURCE_DIR          ${DEP_SOURCE_DIR}/imgui-src
    )
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
    )
endif()

# ----- GLTF Models ----- #
FetchContent_Declare(
    gltf-models
    GIT_REPOSITORY      https://github.com/KhronosGroup/glTF-Sample-Models.git
    GIT_TAG             main
    SOURCE_DIR          ${DEP_SOURCE_DIR}/gltf-models
)
FetchContent_MakeAvailable(gltf-models)

# ----------------------- Geodesy Library ----------------------- #

set(GEODESY_LIBRARY "geodesy")

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
add_library(${GEODESY_LIBRARY} ${INC} ${SRC})

# ======================= PLATFORM COMPILE DEFINITIONS =======================

# Set specific platform macros (only define when true)
if(GEODESY_SYSTEM_WINDOWS)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_SYSTEM_WINDOWS)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC WIN32_LEAN_AND_MEAN)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC _WIN32_WINNT=0x0600)
elseif(GEODESY_SYSTEM_LINUX)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_SYSTEM_LINUX)
elseif(GEODESY_SYSTEM_MACOS)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_SYSTEM_MACOS)
elseif(GEODESY_SYSTEM_IOS)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_SYSTEM_IOS)
elseif(GEODESY_SYSTEM_ANDROID)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_SYSTEM_ANDROID)
endif()

# Set primary platform macros (only define when true)
if(GEODESY_DESKTOP)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_DESKTOP)
else()
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_MOBILE)
endif()

# TODO: This only needs to be set for single-configuration generators
if(GEODESY_BUILD_DEBUG)
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_DEBUG)
else()
    target_compile_definitions(${GEODESY_LIBRARY} PUBLIC GEODESY_RELEASE)
endif()

# ======================= INCLUDE DIRECTORIES =======================

# Core includes (always available)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${Vulkan_INCLUDE_DIRS}/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${DEP_SOURCE_DIR}/)
if(GEODESY_DESKTOP)
    target_include_directories(${GEODESY_LIBRARY} PUBLIC ${glfw_SOURCE_DIR}/include/)
    target_include_directories(${GEODESY_LIBRARY} PUBLIC ${glfw_BINARY_DIR}/src/)
endif()
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${openxr_SOURCE_DIR}/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${stb_SOURCE_DIR}/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${assimp_SOURCE_DIR}/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${assimp_BINARY_DIR}/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${freetype_SOURCE_DIR}/include/)
target_include_directories(${GEODESY_LIBRARY} PUBLIC ${freetype_BINARY_DIR}/include/)
if(GEODESY_DESKTOP)
    target_include_directories(${GEODESY_LIBRARY} PUBLIC ${imgui_SOURCE_DIR}/)
endif()

# Link Against Vulkan
if(OpenMP_CXX_FOUND)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC OpenMP::OpenMP_CXX)
endif()

# Link Against Vulkan
if(Vulkan_FOUND)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC ${Vulkan_LIBRARY})
endif()

# Link Third Party Libraries
if(GEODESY_DESKTOP)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC glfw)
endif()
target_link_libraries(${GEODESY_LIBRARY} PUBLIC openxr_loader)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC glslang)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC SPIRV)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC assimp)
target_link_libraries(${GEODESY_LIBRARY} PUBLIC freetype)

# Platform-specific System Library linking
if(GEODESY_SYSTEM_WINDOWS)
    # Windows specific libraries
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC wsock32 ws2_32)
elseif(GEODESY_SYSTEM_LINUX)
    # Linux specific libraries
    find_package(Threads REQUIRED)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC Threads::Threads)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC dl)
elseif(GEODESY_SYSTEM_MACOS)
    # macOS specific frameworks
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(IOKIT_FRAMEWORK IOKit)
    find_library(COREVIDEO_FRAMEWORK CoreVideo)
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC ${COCOA_FRAMEWORK} ${IOKIT_FRAMEWORK} ${COREVIDEO_FRAMEWORK})
elseif(GEODESY_SYSTEM_IOS)
    # iOS specific frameworks
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC ${UIKIT_FRAMEWORK} ${FOUNDATION_FRAMEWORK} ${METAL_FRAMEWORK} ${METALKIT_FRAMEWORK})
elseif(GEODESY_SYSTEM_ANDROID)
    # Android specific libraries
    target_link_libraries(${GEODESY_LIBRARY} PUBLIC ${log-lib} ${android-lib})
endif()

# ----------------------- Geodesy Unit Test ----------------------- #
# When creating your own project off of the Geodesy Library, use this
# as a example/template for your own CMakeLists.txt file to know what to link against.

set(GEODESY_UNIT_TEST "geodesy-unit-test")

project(${GEODESY_UNIT_TEST})

# Link against Vulkan and OpenMP
add_executable(${GEODESY_UNIT_TEST} main.cpp res/icon.rc)

set_target_properties(${GEODESY_UNIT_TEST} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_SYSTEM_NAME}/${CMAKE_BUILD_TYPE}/)

# Include directories
target_include_directories(${GEODESY_UNIT_TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc/)

# Link Against Geodesy Library
target_link_libraries(${GEODESY_UNIT_TEST} PRIVATE ${GEODESY_LIBRARY})

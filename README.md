[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE.md)
[![forthebadge](https://forthebadge.com/images/badges/works-on-my-machine.svg)](https://forthebadge.com)

# Geodesy Engine

![Alt Text](https://github.com/FermionicChaos/geodesy/blob/master/res/github/logo.png)

The geodesy engine is a high performance game/physics engine written in C++. It utilizes the Vulkan Graphics
& Compute API to interact with GPU hardware. Years ago when I was studying physics and taking a general relativity
class, we were tasked a final project and my class mate and I decided to do a curved spacetime renderer as our
final project. We named the project "Geodesic Renderer", which was a renderer for curved space-time. I had weebified
the name to geodesy, and hence the name for this project was born.

---

# Build Environment Setup
The geodesy engine uses CMake as its build management system, which abstracts over
the various build systems and tools across Linux, Windows, and MacOS. This allows the
engine user to easily setup a ready made build environment for whatever platform they
are operating on and what platform they want to target. There is no build support
for mobile operating systems, and the geodesy engine only compiles out to desktop
platforms. There are no plans to change this.

You will need a series of tools installed to compile and use the geodesy engine. Install
these dependencies first regardless of what platform you are compiling on. These packages
are required across all platforms.

1. [Python](https://www.python.org/downloads/):
You will need to install Python3 to properly use CMake for project/build generation.

2. [CMake](https://cmake.org/download/):
CMake is necessary to generate the proper project files for whatever platform you
wish to compile and run the geodesy engine on. This is critical for generating
the build/project files.

3. [Vulkan](https://www.lunarg.com/vulkan-sdk/):
The Vulkan SDK is necessary to interact with the GPU hardware. Most GPU drivers
package implementations of the Vulkan API in their GPU driver installation package,
and the Vulkan SDK allows you to interact with these driver runtimes to perform
render and compute operations with GPU hardware. The geodesy engine relies on 
the Vulkan API to interact with GPU hardware.

## Windows Environment
If you are using the Windows operating system, it is recommended that you use Visual Studio IDE
for the build process. This Visual Studio IDE has become standard for game development these days,
and when in Rome, act like the Romans. However, there is nothing prohibiting you from using Microsoft's
garbage carbon copy make system (NMake) to build this project. You can also use MinGW, CLang, or whatever
else is supported by CMake. I wouldn't recommend it, so all I will provide as a necessary tool in this
section is a link the Visual Studio IDE. You can use VS Code to edit.

1. [Visual Studio IDE](https://visualstudio.microsoft.com/vs/): The recommended build environment for compiling
this project on Windows is of course Visual Studio IDE. Download this IDE, and install the Destkop C++ development
packages to use the engine. Do not forget to install git with Visual Studio IDE because that is also needed
for this project. You are after all reading this on Github.

2. Project Generation: Execute the following CMake command to generate the project files for Windows OS, and you are good to go.
```bash
cmake -S . -B prj/ -G "Visual Studio 17 2022"
```

3. Project Compilation, Running & Debugging: Because Visual Studio build system allows for multibuild configurations,
this makes it easy for CMake to call the approriate compilers to build both debug and release builds.
```bash
cmake --build prj/ --config Debug
cmake --build prj/ --config Release
```

## Linux Environment
On Linux, you will probably end up using GCC, or CLang, or whatever (IDGAF). You
just need to insure that you have those compilers installed, you need a windowing 
server system to interact with. Either X11 or Wayland, up to you. I don't care. Or
you can use the install.sh script in the doc/ folder to auto install the dependencies.

1. GNU Compiler Collection & Git: 
To compile the project you will need a compiler toolset, and I recommend the classic GCC 
to do the job. You are free to use whatever compiler toolset you like, I will recommend GCC. 
Install GCC by running the command. Git is a versioning system that needs to be installed 
to clone this repository in the first place. If you don't have git, not only will you not 
be able to download this engine, but you can't even download its dependencies. So first 
install Git by running this command.
```bash
sudo apt install gcc g++
sudo apt install git
```

2. Windowing System: You will need a windowing system to interact with. On Linux, there are
two major ones I can think of that you will need to interact with to create a system window
for rendering. The first windowing system is older and more stable, X11. The second is Wayland
which is much newer. I recommend using X11, because I have not messed with Wayland.
```bash
sudo apt install xorg-dev
sudo apt-get install libxcb-glx0-dev
```

3. Project Generation: Due to the nature of Unix Makefiles, you will need to generate
Debug and Release projects to be able to do line-by-line debugging.
```bash
cmake -S . -B prj/Debug -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B prj/Release -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
```
4. Project Compilation, Running & Debugging: Unfortunately the Unix Makefiles system is a single
configuration build system, seperate projects have to be generated for both debugging and release
builds.
```bash
cmake --build prj/Debug
cmake --build prj/Release
```

## MacOS (Darwin)
Throw in trash and choose different computer.

---

# State of the Engine
The geodesy engine is a cross-platform high performance game engine written in C++ utilizing the Vulkan
graphics and compute API to interact with GPU hardware. The game engine is in its early design phases, and
is highly experimental. Efforts have made the engine easy to use on multiple platforms.

## TODO List:

- Create model animation caching system.

- Introduce portaudio to the geodesy engine.

- Add layering system for window objects, for huds, system stats and so
on. (Will be done with canvas class, and window as target.)

- File System stuff.

- Add built in extension types for file.h to recognize file types
and forward to proper objects.

- Add lua support for runtime scripting.

- Add engine asset manager to prevent double loading.

- Add Dynamic Library compilation options.

- Change Texture class to image class? The reasoning behind this change
along with how vulkan does it, is that a texture describes the texture
of a particular surface while an image is a generalized concept of a type
of memory.

## Bug List:

- Figure out gpu uniform buffer update latency

- Figure out window zero size vulkan validation error on windows


# The Geodesy Engine

> **File:** `/docs/design/TheGeodesyEngine.md` **Purpose:** This is the authoritative logic and architecture specification for the Geodesy Engine. It is intended to guide both human development and AI-assisted code generation.

---

## Prologue: Why Geodesy?

I have been working on a game engine myself on and off, for just shy of a decade at this point. What has lead to the development of the Geodesy Engine is that the ideas surrounding the engine are unorthodox enough that it warrants its own creation because of its distinction alone. This is not to say that existing solutions such as Unreal, Unity, and Godot are not excellent engines themselves, it's that we feel that our ideas in architecture are different enough that they warrant the creation of the geodesy engine. There would be no Godot Engine if the developers of the Godot engine said "why make an engine when Unreal and Unity already exist?".

That begs the question, what are the features that I am proposing that warrant the creation of an entirely new engine? For instance, one could say that if a feature is a bad feature, then there is a reason why such a feature hasn't already been implemented in an already existing engine. If a feature is a "good" feature, then what is stopping Epic Games from having it implemented in the Unreal Engine? That is a question worthy of a dignified response. One has to think of a code base as like a forest, and at one point a forest becomes so overgrown that changing it would require burning down the forest and starting all over again. As a code base gets larger and larger, it becomes exponentially more difficult to change the most core aspects of an engine without breaking a build entirely. This is because so much of the code base is reliant on that core code. Thus a developer team becomes entrenched in the code base that already exists. When a code base becomes too large, it becomes far too difficult in time, energy, and ultimately money to change a feature of a code base. In such rare cases the only way to implement a features is to quite literally burn down the whole forest and build an entirely new framework to support said feature.&#x20;

One has to look no further than the developmental differences between the OpenGL API and the Vulkan API. When the specification for OpenGL was first released in the 90s, vertices were streamed to the GPU on a frame by frame basis. It wasn't until the late 2000s did framebuffers, the ability to select other sets of targets to be rendered to for off screen rendering and post processing. Up into the late 2010s, most of the logical primitives of the OpenGL were Ad-hoc. The OpenGL context is tied to a particular system window, and it was agnostic to what GPU on the host system these rendering operations being performed on. It became better to start from scratch with a new API standard provided by AMD (Mantle), and influenced by OpenCL, that could allow high performance usage of GPU hardware.

One of the things I have learned was that the nature of engineering is persistent development and change, and that standards and hardware are always and continuously evolving, and a part of that process is burning down the forest and starting from a new beginning. Does that mean that the intention behind this engine is to dethrone existing engines like Unreal Engine and Unity? Well no, that would be delusional to say that one man could single-handedly make a better engine than Unreal. But that didn't stop the development of the Godot Engine either. By that same token, The Khronos Group doesn't pretend that Vulkan will entirely replace OpenGL either. With all that being said, I think its best to delve into the specific points that make this Engine "different" enough to warrant its own creation.&#x20;

The first being that the Geodesy Engine is not like most traditional engines where you are provided an editor out of the box, in fact the geodesy engine is more like a simulator or runtime environment games are built off of. This includes the editor for the engine, which will be a separate code base entirely built off of the main engine. The idea is that Geodesy is an engine which you build your custom game with assets and C++ code off of to make a game. Meaning, you create your own custom game logic in C++, with some assets that get loaded at runtime, and you compile a game linking against the core engine. &#x20;

What Unreal Engine doesn't do, which I would like the Geodesy Engine to be able to do, is to be able to convert blue print logic, or scripted language logic directly into C++ code which then can be compiled. This is where I believe the Geodesy Engine can shine. What ever scripting language can be trans-coded into C++ code for efficiency in build time of the game.

The next thing the Geodesy Engine will bank on is pure reliance on Vulkan as its API of choice for interacting with GPU hardware. We won't have to worry about compatibility with older legacy APIs, and newer GPU APIs seem to be converging in design. This will allow the game developer fine grained control over GPU hardware unlike legacy APIs. In fact the GPU API is mostly a wrapper for Vulkan anyways.

The largest crucial difference between the design philosophy of the Geodesy Engine, and other engines is that each stage is simply a collection of objects sharing the same space, physics, and rendering. For instance, a stage can hold a collection of objects, such as floors, tables, and other items that can exist, but a camera is also a type of object existing in the same space. It is an object that has the ability to perceive the world, and render it. It is a subject of the stage. This design allows for tremendous modularity in how rendering is done in the Geodesy Engine. You can define a GUI render system that renders directly to a system window, or you can define a stage that is a 3d space with objects like tables, walls, entities, or what ever else one wishes to load, with a camera3d that renders the stage. A subject merely perceives the world it exists in, and can share its resources in other stages, such as canvas which is a 2d space for compositing 2d GUI elements. There is a tremendous amount of modularity with how a user wishes to render a stage. As for the core engine, another bltn (built in) namespace and folder is included with objects like system\_window and camera3d which both define the custom rendering for each specialized subject, but also how to expand C++ code from the core engine code.

Because of the modularity of rendering system, it also leaves room for the development of VR as another extensible render target to be added in and derived from the base subject class. The ultimate goal is to have it where the same 3d space can exist, and you can have a desktop window camera rendering the same 3d space independently of a VR headset.  One should be able to be running a game, and midway running it, start their VR headset without having to restart the game. Thus once again demonstrating the power and utility of focusing on making the rendering system modular.

It should be noted that the intended update thread where all game logic is executed operates on its own time step rather than a fixed frame rate set by any particular render target. This is another perk of the Geodesy Engine over other engines. This allows for multiple render targets (subjects) in each stage to exist independently and be processed at their respective times. Full disclosure, the intention is to have an update thread perform all logic and host operation, then GPU resource updates at a fixed time step (not frame rate), while the render thread acts entirely asynchronously merely only executing to honor the set frame rates of all render targets (subjects) in all stages. We will see if this translates into the desired performance, if we finish the engine in the first place.

I won't lie, I have found an massive appeal in AI. I have spent many years studying game engine design, and poured a good amount of time making this engine. The problem has been scalability. A single man can't continue to write every line of code, and then spend hours of time debugging finding simple errors. That's not time feasible. One thing I have noticed about AI, is it's precise execution of what you ask it to do. It is really good at concretizing well thought out logic into hard code.

Language at the end of the day is meant to express an idea, not be a substitute for good ideas. On top of that most guidelines for AI seem to encourage using pseudo code to generate actual code. Code is only as good as the ideas it comes from. This document serves a two fold purpose. The first it shall contain the logic of the engine in a human readable format to be translated into code later, and second to resolve the contextual loss between user sessions with most AIs available. Context loss seems to be the largest barrier in large scale code development. This document is merely to test the limits of AI in large scale software development. As a lone man I want to finish this project by any means necessary and put it behind me. I've worked on this long enough. I want to move on to other projects in my life.

---

## 1. Coding Standards

### Language and Style

- Language standard: **C++17**
- Use of STL is encouraged, except where custom allocators or containers are necessary.
- RAII and smart pointers (`std::shared_ptr`, `std::unique_ptr`, `std::weak_ptr`) are required for convenient GPU resource management.

### Naming Conventions

- **snake\_case**: For types, functions, and structs.
- **PascalCase**: For variable instances, member variables, and handles.
- Constants are in **UPPER\_SNAKE\_CASE**.
- Stack Allocated Variables are allowed the most freedom in style due to their non persistent nature.

### Code Layout

- Prefer `#pragma once` for simplicity and modern usage.
- Header guards are allowed when more portability or explicit macro naming is desired.
- When using header guards, the macro name must reflect the full file path from the root of the engine source tree.

### Header Guard Convention

If header guards are used instead of `#pragma once`, they must follow the format:

```cpp
#ifndef GEODESY_PATH_TO_HEADER_H
#define GEODESY_PATH_TO_HEADER_H
// Header content...
#endif // GEODESY_PATH_TO_HEADER_H
```

Where `PATH_TO_HEADER_H` is the file path in all uppercase with non-alphanumeric characters replaced by underscores. For example, a file located at:

```
./inc/geodesy/core/gpu/pipeline.h
```

Should have the header guard:

```cpp
#ifndef GEODESY_CORE_GPU_PIPELINE_H
#define GEODESY_CORE_GPU_PIPELINE_H
// ...
#endif // GEODESY_CORE_GPU_PIPELINE_H
```

This ensures all header guards are globally unique and traceable to their file origin.

- Include guards or `#pragma once` on all headers.
- Each namespace maps to a logical module and directory.
- Header/source file pairing per module.

### Directory Layout

- Header/source file pairing per module.

### Sample Class Template

```cpp
// Example header: core/object.h
#pragma once
#ifndef GEODESY_CORE_OBJECT_H
#define GEODESY_CORE_OBJECT_H

#include <math/vec.h>
#include <gfx/model.h>

namespace geodesy::core {

    class object {
    public:
        // Constructors
        object();
        object(const math::vec<float, 3>& Position);

        // Accessors
        const math::vec<float, 3>& position() const;
        void set_position(const math::vec<float, 3>& NewPosition);

        // Public Fields
        std::shared_ptr<gfx::model> Model = nullptr;

    protected:
        math::vec<float, 3> Position;
    };

} // namespace core

#endif // GEODESY_CORE_OBJECT_H
```

This sample illustrates the Geodesy Engine’s code conventions:

- Class name `object` is in `snake_case`, matching the engine's naming convention for types.
- Member variables like `Position` are `PascalCase`.
- Public members are clearly separated from protected/private ones.
- Header guards use `#pragma once`.
- Includes are minimal and module-specific.
- You can name stack variables what ever you like.&#x20;

Further examples for more complex systems will follow as modules are built.

---

## 2. Core Utilities

The core utilities of the Geodesy serve as the base elements which objects are constructed from. For instance, the gfx::model class is built on the gpu::buffer class, and there is certainly some hierarchy to the core modules. The gfx::model class serves as the graphical model of the object in rendering. The math module serves as the base module class for vectors, matrices, quaternions, and a specialized field class for vector fields. An object position is a math::vec\<float, 3>.

Core Engine Sub-modules

- io (File input/output & Asset management)
- math (Mathematics)
- lgc (Logic Primitives)
- phys (Physics)
- hid (Human Interface Device)
- gpu (GPU Hardware Interface API)
- gfx (Rendering & Graphics Module)
- sfx (Sound Effects & Processing)

### `io` Module

The `io` module is the runtime-facing asset loader system of the Geodesy Engine. It is built to support a wide variety of host-resident asset types, while maintaining efficiency through shared memory control and future hooks for GPU residency.

#### `io::file` (Base File Interface)

- All runtime asset types are extensions of `io::file`.
- This includes:
  - 3D models
  - Shader source code
  - Image files (textures)
  - Sound files
  - Fonts / typeface definitions
  - Any other loadable file asset
- Each file is loaded into **host RAM** and kept in managed `weak_ptr` form.
- Derived file types implement custom loaders to parse and populate internal memory.

#### `gpu::image` and `gpu::shader` Derivation

- Both `gpu::image` and `gpu::shader` classes **inherit from \*\*\*\*\*\*\*\*\*\*\*\***\`\`.
- This provides a consistent interface for asset memory handling.
- Once loaded into host memory, they can be staged into device memory using the `gpu::context` system.

#### `io::file::manager`

- Central registry that ensures host assets are not loaded redundantly.
- Uses shared pointers to guarantee a single copy of any unique file.
- Allows multiple systems to refer to the same file without conflict.

#### Planned: Host ↔ GPU Residency Map

- The design anticipates a future feature linking loaded host assets with GPU memory.
- A **map of weak pointers** to file assets will link to their GPU-side allocations.
- This enables automatic unloading of host assets from RAM once uploaded to device memory (via `gpu::context`).
- Ideal for large games where RAM conservation is essential.

#### Design Philosophy

- Asset-type agnostic
- Smart memory reuse
- Prepared for staged host→device transitions
- Extensible: loading logic is delegated to derived file types

The `io` module is the runtime-facing asset loader system of the Geodesy Engine. It is built to support a wide variety of host-resident asset types, while maintaining efficiency through shared memory control and future hooks for GPU residency.

#### `io::file` (Base File Interface)

- All runtime asset types are extensions of `io::file`.
- This includes:
  - 3D models
  - Shader source code
  - Image files (textures)
  - Sound files
  - Fonts / typeface definitions
  - Any other loadable file asset
- Each file is loaded into **host RAM** and kept in managed `weak_ptr` form.

#### `io::file::manager`

- Central registry that ensures host assets are not loaded redundantly.
- Uses shared pointers to guarantee a single copy of any unique file.
- Allows multiple systems to refer to the same file without conflict.

#### Planned: Host ↔ GPU Residency Map

- The design anticipates a future feature linking loaded host assets with GPU memory.
- A **map of weak pointers** to file assets will link to their GPU-side allocations.
- This enables automatic unloading of host assets from RAM once uploaded to device memory (via `gpu::context`).
- Ideal for large games where RAM conservation is essential.

#### Design Philosophy

- Asset-type agnostic
- Smart memory reuse
- Prepared for staged host→device transitions

The `io` system ensures robust runtime asset handling and memory conservation, forming the bridge between disk and device.

---

### `math` Module

The `math` module provides the foundational numeric and geometric types for all simulation, rendering, and spatial operations in the Geodesy Engine. It is designed to be lean, readable, and performant, avoiding unnecessary overhead while retaining clarity and usability.

#### Core Features

- `vec<T, N>`: Generic N-dimensional vector template with overloaded arithmetic, dot/cross products, normalization, and swizzle operations.
- `mat<T, N>`: N×N matrix template supporting affine transforms, rotations, inverses, and multiplication.
- `quaternion<T>`: Quaternion representation with normalization, rotation application, and spherical interpolation.
- `field<T>`: A templated field class representing N-dimensional domains of data. Used for modeling scalar/vector fields like heightmaps or 3D volumes.
- `complex<T>`: Complex number type useful for 2D transformations or frequency domain calculations.

#### Module Details

- All types are fully templated and designed to be interoperable (e.g. `mat<float, 3>` can multiply `vec<float, 3>`).
- Vector types (`vec`) are frequently used in objects for position, orientation (as `quat`), momentum, and forces.
- Matrix operations (`mat`) are used extensively in the GPU pipeline for camera, model, and projection transforms.
- Quaternion logic (`quat`) provides robust 3D rotation handling without suffering from gimbal lock.
- Constants like `PI`, `E`, and `TAU` are defined in `constants.h`.
- The library defines type aliases such as `vec3f`, `mat4f`, and `quatf` for convenience.

#### Internal Utilities

- `config.h`: Controls internal type precision (e.g. `math::real` resolves to `float` or `double`).
- `type.h`: Defines internal base types and mathematical traits.
- `func.h`: Contains numerical functions like `clamp()`, `lerp()`, `smoothstep()`.

#### Design Philosophy

- Deterministic and low-overhead math for simulation.
- Templated for flexibility but designed with clarity.
- Interoperable with Vulkan coordinate systems and GPU buffer types.

The math module is considered stable and will only be revised for clarity, optimization, or bug fixing. It forms the bedrock of all vector/matrix/quaternion computations in the engine.

- `vec`, `mat`, `quat` implementations.
- Coordinate transforms and matrix operations.

### lgc (Logic Primitives)

The `lgc` module defines the behavioral logic system in Geodesy. It provides the foundation for object-based and stage-aware simulation logic. At its core lies the `behavior` class, which all runtime logic derives from.

#### `behavior` (Base Logic Interface)

- Defines overridable virtual functions for runtime events:
  - `on_creation(object*, stage*)`
  - `on_update(object*, stage*, float dt)`
  - `on_event(const event&, object*, stage*)`
  - `on_destruction(object*, stage*)`
- Attached to any `runtime::object` or `runtime::subject`.
- Receives simulation time updates, physics events, custom messages, and stage lifecycle triggers.

#### `scripted_behavior` (Derived Logic Implementation)

- Inherits from `behavior`.
- Loads runtime logic from external sources such as Lua scripts or visual graph representations.
- Implements all base callbacks by executing script-defined behavior logic.
- Exposes internal state and engine hooks to scripting backend via registered bindings.

##### Use Case:

- Rapid prototyping.
- Hot-reloadable behaviors.
- Visual node graph editing.

#### `compiled_behavior` (Transpiled Graph Logic)

- Also derived from `behavior`.
- Generated from visual scripting graphs or logic DSLs.
- Transpiled into C++ code during build step for native performance.

#### Design Philosophy:

- All behavior logic is ultimately unified under the `behavior` interface.
- The engine does not distinguish between scripted, visual, or compiled logic at runtime—it simply calls the correct virtual methods.
- This supports experimentation in scripting and graph systems, while enabling promotion to native code for optimized builds.

#### Future Concepts:

- Logic composition via behavior stacks or chains.
- Behavior registries for dynamic loading.
- A node graph editor producing `scripted_behavior` DSLs or Lua scripts.
- Custom serialization for debugging, replay, and AI training.

### phys (Physics Submodule)

The `phys` module defines the physical representation of objects in the engine and governs their interaction with space, forces, and collisions. Its structure is built upon a recursive node hierarchy to match that of the `gfx` module, ensuring a unified spatial system for both rendering and physics.

### `phys::node`

- Defines spatial hierarchy for physics objects.
- Contains:
  - `Position`: math::vec3
  - `Orientation`: math::quat
  - `Scale`: math::vec3
  - `Parent` and `Children` pointers
  - Cached `WorldTransform`
- Used for recursive transform updates, physics hierarchy processing, and attachment logic.

### `phys::mesh`

- A CPU-side physics mesh representation used for collision detection and structural analysis.
- Contains an internal `vertex` struct that defines the **Geodesy Engine's standard vertex format**, including:
  - Position (math::vec3)
  - Normal (math::vec3)
  - Tangent (math::vec3)
  - Bitangent (math::vec3)
  - Texture Coordinates (math::vec2)
  - Bone Weights (float[4])
  - Bone Indices (int[4])
- Mesh data includes triangle indices and vertex lists, used for both rendering and physics.
- Bone data enables skeletal deformation support and is defined per vertex.
- This format is shared between `phys::mesh` and `gfx::mesh` to ensure seamless translation between CPU simulation and GPU rendering.

### Design Intent

- `gfx::mesh` and `gfx::node` are derived from `phys::mesh` and `phys::node`, respectively.
- This allows render-facing objects to inherit all physical positioning and hierarchy logic.
- `object` will store its instance transform tree using `gfx::node` instances, which carry physics structure through `phys::node`.

The `phys` module provides the foundational spatial logic for all simulation and rendering integration. Future expansion will incorporate rigid body dynamics, constraint solvers, and scene collision structures.

---

### HID

The `hid` module (Human Interface Devices) will manage all platform-specific input code, abstracting access to:

- Mouse
- Keyboard
- Gamepad
- VR controllers
- Other user-driven or sensor-based input devices

This module is highly platform-dependent and will eventually bridge native OS input handling to Geodesy's internal event system.

#### Implementation Status:

This section is currently a placeholder. A full input routing and forwarding system—where runtime `object`s can receive and respond to input—has yet to be designed. Input event forwarding, focus control, and input mapping are future features to be specified in later revisions.

- Abstract input layer for mouse, keyboard, gamepad, and VR input.
- Event system decouples input from objects.

---

## 3. Geodesy Runtime

### Physics Integration

After all gameplay logic has been applied and each object in the stage is evolved forward in time (via input, behavior, and other simulation), the next natural step in the update thread is the **physics resolution stage**.

At this point, all forces, velocities, and position updates have already been applied. This is the ideal time to evaluate **physical interactions** such as:

- **Collision Detection**: Checking if any objects have intersected after motion.
- **Constraint Resolution**: Resolving overlapping objects and applying response forces.
- **Trigger Volumes**: Detecting entry or exit from defined zones.
- **Impulse Response**: Updating momentum or state based on collisions.

This separation of logic-then-physics ensures that behavior-driven motion does not conflict with physical laws, and that the engine can process a clean, deterministic collision state.

The physics pass is stateless between frames except for persistent dynamics (e.g., velocity and forces), and it generates **collision events** that are dispatched to the `behavior` system or other relevant callbacks.

Future expansions of the `phys` module may include:

- Rigid body simulation
- Kinematic constraints
- Gravity zones
- Soft body or fluid models
- VR collision proxy integration

Explanation of Object/Subject/Stage/App. The runtime primitives recognized and processed by the engine are the base classes object, subject, stage, and app. Some of the methods of these classes can be overridden when developing an app from the engine base code. The base object class is the world primitive that exists. It has qualities such as position, orientation, and scale.

The option table is a table of runtime options of how object gets processed by the geodesy engine.

option

- IsRenderable
- IsCollidable

object

- World
- Motion {STATIC, DYNAMIC, KINEMATIC, ANIMATED}
- Option
- Time
- Mass
- Position
- Orientation
- Scale
- Linear Momentum
- Angular Momentum
- Behavior
- Model
- input(?)
- update(DeltaTime)
- draw(Subject)

Subject is a special type of object that renders the stage it exists in. For example, a camera3d is a type of subject that renders 3d environments. A system\_window is a type of window which renders mostly 2d GUI graphics, but utilizes the same rendering API to do so.

subject : object

- Subject should be synonymous with render target. That's all the subject class is.
- A subject is a special type of object capable of perceiving and rendering its surrounding environment.
- It is **not** necessarily the player or agent—it may be a camera, a VR headset feed, or a window-based rendering system.
- The term 'subject' refers to its role in rendering the scene.
- Rendering in Geodesy is modular—each subject may define its own render target and method (e.g., offscreen render, GUI stage render, VR compositor).
- The engine may contain multiple subjects in one or many stages, each observing or displaying the same or different views of the world, and sharing its render results in other stages.

The way the Geodesy Engine processes these primitives is as follows. There exists the update thread which is logically the main game loop of the engine. It polls user input, processes user input, executes game logic, updates all stages in existence, and all objects in these stages. It then performs the proper physics such as collision.&#x20;

Update Thread (Main Thread)

1. **Poll User Input** – Platform-specific input polling (mouse, keyboard, gamepad, VR, etc.).
2. **Distribute Input** – Input is forwarded to objects marked `input()`.
3. **Apply Game Logic** – Each `stage` is updated:
   - Calls `update(dt)` for all `object`s.
   - Executes attached behaviors and timers.
   - Processes queued or conditional events (e.g., health-based transitions).
4. **Apply Motion Updates** – Transform updates are applied based on game logic outputs.
5. **Run Physics Simulation** – Physics engine computes:
   - Collision detection
   - Rigid body dynamics
   - Constraint resolution
   - Trigger volume overlaps
6. **Dispatch Collision Events** – `on_collision_enter`, `on_trigger_enter`, etc., are dispatched.
7. **Update GPU Resources** – Synchronizes host-side data with GPU state:
   - Uploads new transforms, materials, animation states, etc.
   - Finalizes staging buffers and descriptor writes for rendering.

Render Thread (Main Thread)

The render thread is a completely separate thread, and unlike the update thread, has no fixed time step. Instead the render thread is called repeatably to check on every render target (subject) in existence in each stage. It checks each render target (subject) to see if it is ready to perform rendering operations and then aggregates all GPU command buffers to be submitted and executed on the proper parent GPU. It iterates though each stage, and each render target (subject) in each stage for rendering operations.

---

## 6. How to Use the Engine

> This section will serve as a future user guide for developers using Geodesy.

- Creating a new `core::app`
- Building and populating `core::stage`
- Defining custom rendering systems in `core::subject`
- Integrating raw Vulkan calls
- Editing logic via visual scripting
- Compiling runtime logic to native code

---

*This document will expand as modules are designed, implemented, and refined. Each section is a contract for both developers and AI collaborators.*


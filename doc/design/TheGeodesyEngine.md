# The Geodesy Engine

> **File:** `/docs/design/TheGeodesyEngine.md`
> **Purpose:** This is the authoritative logic and architecture specification for the Geodesy Engine. It is intended to guide both human development and AI-assisted code generation.

---

## Prologue: Why Geodesy?

I have been working on a game engine myself on and off, for just shy of a decade at this point. What has lead to the development of the Geodesy Engine is that the ideas surrounding the engine are unorthodox enough that it warrants its own creation because of its distinction alone. This is not to say that existing solutions such as Unreal, Unity, and Godot are not excellent engines themselves, it's that we feel that our ideas in architecture are different enough that they warrant the creation of the geodesy engine. There would be no Godot Engine if the developers of the Godot engine said "why make an engine when Unreal and Unity already exist?".

That begs the question, what are the features that I am proposing that warrant the creation of an entirely new engine? For instance, one could say that if a feature is a bad feature, then there is a reason why such a feature hasn't already been implemented in an already existing engine. If a feature is a "good" feature, then what is stopping Epic Games from having it implemented in the Unreal Engine? That is a question worthy of a dignified response. One has to think of a code base as like a forest, and at one point a forest becomes so overgrown that changing it would require burning down the forest and starting all over again. As a code base gets larger and larger, it becomes exponentially more difficult to change the most core aspects of an engine without breaking a build entirely. This is because so much of the code base is reliant on that core code. Thus a developer team becomes entrenched in the code base that already exists. When a code base becomes too large, it becomes far to difficult in time, energy, and ultimately money to change a feature of a code base. In such rare cases the only way to implement a features is to quite literally burn down the whole forest and build an entirely new framework to support said feature.&#x20;

One has to look no further than the developmental differences between the OpenGL API and the Vulkan API. When the specification for OpenGL was first released in the 90s, vertices were streamed to the GPU on a frame by frame basis. It wasn't until the late 2000s did framebuffers, the ability to select other sets of targets to be rendered to for off screen rendering and post processing. Up into the late 2010s, most of the logical primitives of the OpenGL were Ad-hoc. The OpenGL context is tied to a particular system window, and it was agnostic to what GPU on the host system these rendering operations being performed on. It became better to start from scratch with a new API standard provided by AMD (Mantle), and influenced by OpenCL, that could allow high performance usage of GPU hardware.

One of the things I have learned was that the nature of engineering is persistent development and change, and that standards and hardware are always and continuously evolving, and a part of that process is burning down the forest and starting from a new beginning. Does that mean that the intention behind this engine is to dethrone existing engines like Unreal Engine and Unity? Well no, that would be delusional to say that one man could single-handedly make a better engine than Unreal. But that didn't stop the development of the Godot Engine either. By that same token, The Khronos Group doesn't pretend that Vulkan will entirely replace OpenGL either. With all that being said, I think its best to delve into the specific points that make this Engine "different" enough to warrant its own creation.&#x20;

The first being that the Geodesy Engine is not like most traditional engines where you are provided an editor out of the box, in fact the geodesy engine is more like a simulator or runtime environment games are built off of. This includes the editor for the engine, which will be a separate code base entirely built off of the main engine. The idea is that Geodesy is an engine which you build your custom game with assets and C++ code off of to make a game. Meaning, you create your own custom game logic in C++, with some assets that get loaded at runtime, and you compile a game linking against the core engine. &#x20;

What Unreal Engine doesn't do, which I would like the Geodesy Engine to be able to do, is to be able to convert blue print logic, or scripted language logic directly into C++ code which then can be compiled. This is where I believe the Geodesy Engine can shine. What ever scripting language can be trans-coded into C++ code for efficiency in build time of the game.

The next thing the Geodesy Engine will bank on is pure reliance on Vulkan as its API of choice for interacting with GPU hardware. We won't have to worry about compatibility with older legacy APIs, and newer GPU APIs seem to be converging in design. This will allow the game developer fine grained control over GPU hardware unlike legacy APIs. In fact the GPU api is mostly a wrapper for Vulkan anyways.

The largest crucial difference between the design philosophy of the Geodesy Engine, and other engines is that each stage is simply a collection of objects sharing the same space, physics, and rendering. For instance, a stage can hold a collection of objects, such as floors, tables, and other items that can exist, but a camera is also a type of object existing in the same space. It is an object that has the ability to perceive the world, and render it. It is a subject of the stage. This design allows for tremendous modularity in how rendering is done in the Geodesy Engine. You can define a GUI render system that renders directly to a system window, or you can define a stage that is a 3d space with objects like tables, walls, entities, or what ever else one wishes to load, with a camera3d that renders the stage. A subject merely perceives the world it exists in, and can share its resources in other stages, such as canvas which is a 2d space for compositing 2d GUI elements. There is a tremendous amount of modularity with how a user wishes to render a stage. As for the core engine, another bltn namespace and folder is included with objects like system\_window and camera3d which both define the custom rendering for each specialized subject, but also how to expand C++ code from the core engine code.

Because of the modularity of rendering system, it also leaves room for the development of VR as another extensible render target to be added in and derived from the base subject class. The ultimate goal is to have it where the same 3d space can exist, and you can have a desktop window camera rendering the same 3d space independently of a VR headset.  One should be able to be running a game, and midway running it, start their VR headset without having to restart the game. Thus once again demonstrating the power and utility of focusing on making the rendering system modular.

It should be noted that the intended update thread where all game logic is executed operates on its own time step rather than a fixed frame rate set by any particular render target. This is another perk of the Geodesy Engine over other engines. This allows for multiple render targets (subjects) in each stage to exist independently and be processed at their respective times. Full disclosure, the intention is to have an update thread perform all logic and host operation, then GPU resource updates at a fixed time step (not frame rate), while the render thread acts entirely asynchronously merely only executing to honor the set frame rates of all render targets (subjects) in all stages. We will see if this translates into the desired performance, if we finish the engine in the first place.

I won't lie, I have found an massive appeal in AI. I have spent many years studying game engine design, and poured a good amount of time making this engine. The problem has been scalability. A single man can't continue to write every line of code, and then spend hours of time debugging finding simple errors. That's not time feasible. One thing I have noticed about AI, is it's precise execution of what you ask it to do. It is really good at concretizing well thought out logic into hard code, or&#x20;



Language at the end of the day is meant to express an idea, not be a substitute for good ideas. Code is only as good as the ideas it comes from. This document serves a two fold purpose. The first it shall contain the logic of the engine in a human readable format to be translated into code later, and second to resolve the contextual loss between user sessions with most AIs available. Context loss seems to be the largest barrier in large scale code development. This document is merely to test the limits of AI in large scale software development.&#x20;





Geodesy is a next-generation game engine focused on high-performance simulation, hybrid GPU rendering, and runtime-defined logic. Unlike Unity or Unreal, Geodesy:

- Directly exposes the Vulkan API for ultimate control and performance.
- Separates simulation from rendering cleanly.
- Uses C++ as its primary expression language, but defers logic to runtime systems.
- Supports both traditional desktop and VR rendering pipelines simultaneously.

Geodesy is not a game engine that hides complexity—it’s a simulation toolchain designed for precision, flexibility, and raw speed. It is built for developers who need deterministic execution and transparent architecture.

### Philosophy of Simulation and Objecthood

Geodesy treats simulation as primary, and rendering as secondary. It draws a clear line between what exists, what acts, and what is seen:

- **Objects** are entities with a position and identity, but no internal agency.
- **Subjects** are objects that carry intent or logic—capable of acting.
- **Stages** are environments or containers that simulate and evolve systems.
- **Apps** orchestrate the engine: they present, loop, and coordinate stages.

Rendering is a downstream consequence of simulation—it is the perceptual layer, not the essence of the system. Objects exist whether they are seen or not.

This inversion of rendering-first priorities is fundamental to Geodesy's identity. It is a reality-modeling engine more than a rendering one.

---

## 1. Coding Standards

### Language and Style

- Language standard: **C++17**
- Use of STL is encouraged, except where custom allocators or containers are necessary.
- RAII and smart pointers (`std::shared_ptr`, `std::unique_ptr`, `std::weak_ptr`) are required for resource management.

### Naming Conventions

- **snake\_case**: For types, functions, and structs.
- **PascalCase**: For variable instances, member variables, and handles.
- Constants are in **UPPER\_SNAKE\_CASE**.

### Code Layout

- Header/source file pairing per module.
- Each namespace maps to a logical module.
- Include guards or `#pragma once` on all headers.

---

## 2. Core Utilities

These are building blocks used across all major systems:

### `math` Module

- `vec`, `mat`, `quat` implementations.
- Coordinate transforms and matrix operations.

### `gpu` Module

- Abstractions over Vulkan: `gpu::buffer`, `gpu::image`, `gpu::pipeline`, etc.
- Smart-pointer-wrapped GPU resource management.

### Asset and File Systems

- `asset_manager` handles loading, reference counting, and unloading.
- File paths are virtualized and resolved through an internal mount system.

### IO

- Abstract input layer for mouse, keyboard, gamepad, and VR input.
- Event system decouples input from objects.

---

## 3. Base Engine Objects

These are the composable types that represent entities within the world:

### `core::position`

- World transform (translation, rotation, scale).

### `core::model`

- Mesh hierarchy + material bindings.

### `core::object`

- Composed of `position` + `model` + runtime logic.
- Exists within a stage but does not act on its own.

### `core::subject`

- Subclass of `object` that carries behavior and decision logic.
- Defined through script, visual graph, or compiled C++ modules.
- Acts within the world, changing state through logic systems.

### `core::stage`

- A container of simulation.
- Manages objects and subjects, runs systems, enforces determinism.

### `core::app`

- Wraps the window, input, frame loop, and rendering orchestration.
- Hosts and transitions between stages.

### Observer/Participant Distinction

- Systems are built to differentiate between **participants** (entities being simulated) and **observers** (interfaces perceiving simulation state).
- Enables rendering, debugging, and UI layers without mutating the simulation.

---

## 4. Execution Model

- **Frame Loop**: Systems iterate over objects each frame.
- **Runtime Logic**: Defined using compiled scripting or graph-based logic.
- **Parallelism**: Wherever possible, systems execute concurrently.
- **GPU Submissions**: Handled by a `render_graph` with dependency resolution.

---

## 5. Visual Scripting and Runtime Behavior

- Logic trees or behavior graphs define dynamic behaviors.
- Editor runs logic in an interpreter.
- Final builds convert graphs into native C++.

---

## 6. How to Use the Engine

> This section will serve as a future user guide for developers using Geodesy.

- Creating a new `core::app`
- Building and populating `core::stage`
- Attaching behaviors to `core::subject`
- Integrating raw Vulkan calls
- Editing logic via visual scripting
- Compiling runtime logic to native code

---

*This document will expand as modules are designed, implemented, and refined. Each section is a contract for both developers and AI collaborators.*


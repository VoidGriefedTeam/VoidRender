# VoidRender

### 3D Graphics Made for Beginners.

VoidRender is a **C++23 3D rendering engine** built around a custom rendering pipeline, live camera controls, custom map formats, and GPU-accelerated computation.

The project combines OpenGL drawing through SimpleGL with OpenCL computation through VoidCLcompute, creating a rendering system designed to make working with 3D scenes more accessible.

**Current Version:** `v1.5.3 Beta`

---

## Rendering Demo

See VoidRender render a 3D scene in real time.

<video src="assets/example.mp4" controls muted playsinline preload="metadata" width="100%">
  Your browser does not support embedded video. [Watch the rendering demo](assets/example.mp4).
</video>

[Open the rendering video](assets/example.mp4)

---

## Table of Contents

- [Features](#features)
- [Getting Started](#getting-started)
- [Configuration](#configuration)
- [Maps and MAP2](#maps-and-map2)
- [Rendering Architecture](#rendering-architecture)
- [Dependencies](#dependencies)
- [Building from Source](#building-from-source)
- [Project Structure](#project-structure)
- [Roadmap](#roadmap)
- [Contributing](#contributing)
- [License](#license)

---

## Features

### 3D Rendering

- 3D objects represented by vertices and faces.
- Perspective projection.
- Per-vertex colors and color gradients.
- Configurable rendering window and background.
- Reusable rendering library.

### Live Camera

- Real-time camera movement.
- Live camera rotation.
- Configurable field of view.
- Camera-relative perspective rendering.

### Custom Map System

- Custom map representation for 3D scenes.
- MAP2 format with a dedicated specification.
- Binary map loading through `mapfmt`.
- MAP1 loading compatibility in the MAP2 development line.

### GPU Computation

- OpenGL-based drawing through SimpleGL.
- OpenCL-based computation through VoidCLcompute.
- Separate libraries for drawing, computation, and rendering.

---

## Getting Started

### Requirements

VoidRender is developed for a C++23 environment. To build the project, you will generally need:

- Windows and a compatible graphics driver.
- A C++23 compiler, such as MSVC.
- CMake 4.4 or a compatible newer release.
- Ninja or another supported build system.
- OpenGL support.
- An OpenCL runtime compatible with VoidCLcompute.
- The required project dependencies.

A compatible integrated GPU may be sufficient; dedicated graphics hardware is not inherently required. Actual performance depends on the hardware, drivers, and workload.

### Clone the Repository

```powershell
git clone https://github.com/VoidGriefedTeam/VoidRender.git
cd VoidRender
```

### Configure and Build

For a Ninja-based build:

```powershell
cmake -S . -B build -G Ninja
cmake --build build --config Release
```

Make sure the dependencies required by the current CMake configuration are available before building.

### Run an Example

Build the desired example target and launch its generated executable. The exact target names and executable paths depend on the repository's current CMake configuration.

Ensure that the runtime DLLs required by the application can be found by Windows.

---

## Configuration

VoidRender uses JSON configuration for window settings.

For example:

```json
{
  "window": {
    "width": 640,
    "height": 360,
    "title": "VoidRender",
    "resizable": true,
    "bgcolor": [0, 0, 0]
  }
}
```

This configuration defines the window dimensions, title, resizing behavior, and background color.

The available settings depend on the configuration structures implemented by the current version.

---

## Maps and MAP2

VoidRender uses a custom map system to represent 3D scenes.

Scene data can describe objects through their vertex coordinates, colors, rotations, and faces, along with camera information.

### MAP2 Documentation

The MAP2 format has its own dedicated documentation:

**[Read the MAP2 Guide](MAP2-guide.md)**

Refer to this guide for the format's specification and supported data structures rather than relying on assumptions about the file format.

### Map Compilation Workflow

The map tooling supports a text-to-binary workflow:

```text
Text map (.map)
      |
      v
    mapc
      |
      v
 Binary map
      |
      v
 mapfmt loader
      |
      v
 VoidRender scene
```

The exact compilation and loading commands are documented alongside the map-format tooling.

### MAP1 Compatibility

The MAP2 development line supports loading compatible MAP1 data. MAP2 is the current format being developed for VoidRender.

---

## Rendering Architecture

VoidRender is built from separate components, each with a specific responsibility.

```text
          Application
               |
               v
           VoidRender
          /          \
         v            v
     SimpleGL     VoidCLcompute
         |            |
         v            v
      OpenGL         OpenCL
         |            |
         v            v
     Graphics      Computation
```

### SimpleGL

SimpleGL is a simplified OpenGL drawing library used by VoidRender for rendering operations.

### VoidCLcompute

VoidCLcompute is a custom OpenCL computation library used to provide GPU-computation functionality.

### VoidRender

VoidRender brings the components together with scene data, camera handling, perspective projection, and map loading.

---

## Dependencies

| Component | Purpose |
|---|---|
| C++23 | Main programming language |
| SimpleGL | OpenGL drawing |
| VoidCLcompute | OpenCL computation |
| `mapfmt` | Custom map loading |
| Glaze | JSON configuration parsing |
| CMake | Build-system configuration |

SimpleGL and VoidCLcompute are separate projects developed for use in the VoidRender ecosystem.

---

## Project Structure

The repository's exact layout may change as development continues. Its main conceptual components include:

| Component | Responsibility |
|---|---|
| Rendering library | Core rendering functionality |
| Map-format tooling | Map compilation and loading |
| Configuration | Window and rendering settings |
| Examples | Demonstrations of the engine |
| `assets/` | Demo media and other assets |
| `MAP2-guide.md` | MAP2 format documentation |

The rendering demonstration:

https://github.com/user-attachments/assets/76c3b2bb-9533-4d78-8fc5-2c4cf45fb588

If you are having some issues with the video above try the rendering demonstration available at [`assets/example.mp4`](assets/example.mp4).

---

## Roadmap

Potential areas of continued development include:

- Expanding the MAP2 scene format.
- Developing lighting capabilities.
- Improving scene visibility and culling.
- Expanding camera and object transformation controls.
- Improving rendering quality and performance.
- Simplifying the application-facing API.
- Adding more examples and documentation.

This roadmap describes development directions, not a guarantee that every item is implemented in the current release.

---

## Contributing

Contributions, bug reports, and suggestions are welcome.

When reporting a bug, include relevant details such as:

- Your operating system.
- Compiler and build-system versions.
- Hardware and graphics/compute driver information.
- Steps to reproduce the problem.
- Relevant build logs or runtime output.

For changes involving map files, consult [`MAP2-guide.md`](MAP2-guide.md) first.

---

## License

See the repository's `LICENSE` file for the applicable license and usage terms.

---

**VoidRender — 3D graphics, made more approachable.**

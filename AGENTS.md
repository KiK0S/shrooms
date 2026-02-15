# AGENTS.md

Discussion of the design and architecture of this engine can be found here:
https://chatgpt.com/share/695a72b0-d108-800e-ae82-b51d4c3081f5

## Purpose of this repository

This repository is an experimental **C++-first, web-first engine** that supports **both step-based
visualization apps** (algorithm demos, chess UIs) and **continuous real-time apps** (games,
simulations) **in the same codebase**.

The goal is:

* **One application interface**
* **One rendering contract**
* **Two execution modes** (step + realtime)
* **Two rendering styles** (UI-only + multi-pass WebGL)

If you change anything, preserve these invariants.

---

## Core architectural invariants (DO NOT BREAK)

### 1. `Frame` is the only rendering contract

All rendering flows through:

```cpp
renderer.draw(const Frame&);
```

A `Frame` may contain:

* `UIStream` (2D immediate primitives)
* `RenderPlan` (multi-pass rendering plan)

Renderers **consume frames**, never Scenes or ECS directly.

---

### 2. Retained state != render output

* Retained state (Scene, ECS, widgets, simulation) **stays private**.
* Per-frame render output is **rebuilt every frame** into:
  * `UIStream`
  * `RenderPlan`

Do **not** serialize Scenes.
Do **not** make renderers traverse Scenes.

---

### 3. Input is event-driven, not callback-driven

* Platform layers push `InputEvent`s into an `InputQueue`.
* Apps and UI **pull events during update**.
* No direct JS callbacks into game logic.
* Determinism and replay must remain possible.

---

### 4. Apps do not know how often they run

All applications implement:

```cpp
class IApp {
  void init();
  void update(const AppContext&, std::span<const InputEvent>, Frame& out);
};
```

They must work in:

* **Step mode** (explicit step calls)
* **Realtime mode** (continuous loop)

Drivers decide *when* `update()` runs.

---

### 5. No engine logic in platform code

Platform layers (web/native) are responsible only for:

* window/canvas creation
* timing
* input collection
* presenting rendered output

They must **not** contain:

* gameplay logic
* rendering logic beyond backend glue
* UI layout or widgets

---

## Current runtime architecture

### Platform + Driver + Renderer

* **Platform** creates window/canvas, owns input pump, provides timing and present.
* **Driver** owns the loop (or step call) and calls `update()` with events each tick.
* **Renderer** consumes `Frame` only.

Platforms do **not** run loops; drivers do.

### Key interfaces (current)

* `engine::IPlatform` in `libs/platform/include/engine/platform.h`
  * `init`, `create_renderer`, `pump_events`, `make_context`, `present`
* `engine::StepDriver` and `engine::RealtimeDriver`
  * `libs/platform/include/engine/drivers.h`
  * `libs/platform/src/step.cpp`, `libs/platform/src/realtime.cpp`
* `engine::IRenderer` + `set_view_size` + `set_clear_color`
  * `libs/render/include/engine/renderer.h`

### ECS split

* `ecs::EcsLogic` holds ECS init + tick (`libs/ecs/driver.hpp`)
* `ecs::EcsStepDriver` / `ecs::EcsRealtimeDriver` drive ECS logic using the driver base.
* Regular apps can use `engine::AppStepDriver` / `engine::AppRealtimeDriver`.

---

## Repository structure (current, relevant)

```
/libs
  /core        (app/input/frame/resource ids)
  /ecs         (ECS driver + init)
  /systems     (domain systems: render, collision, input, etc.)
  /platform    (platform interface + SDL/Emscripten impl + drivers)
  /render
    /canvas2d  (UI-only renderer)
    /webgl     (WebGL renderer)
  /ui          (UIStream, widgets/layout helpers)
  /utils       (arena, file system, misc helpers)
  /edulcni     (demo helpers)

/apps
  /step_demo
  /edulcni_demo
  /particles_demo
  /unified_demo
  /ecs_wasd_demo
  /shrooms_demo

/assets

/third_party
  /edulcni   (read-only reference)
  /shrooms   (read-only reference)
```

---

## Rendering model (very important)

### UI-only rendering

* Uses `UIStream`
* Rendered via Canvas2D or GL-2D batching
* Used for:
  * algorithm visualization
  * chess boards
  * menus
  * editor UI
  * debug overlays

### Real-time rendering

* Uses `RenderPlan`
* `RenderPlan` contains ordered `PassPacket`s
* Each `PassPacket` contains sorted draw items (IDs, not pointers)

### Hybrid

Most real apps use:

* `RenderPlan` for world
* `UIStream` for overlay

---

## How to add a new demo or game

1. Create a new directory in `/apps/<name>`.
2. Implement `IApp` or `ecs::EcsLogic`.
3. Pick driver:
   * `AppStepDriver` / `AppRealtimeDriver`
   * `EcsStepDriver` / `EcsRealtimeDriver`
4. Use a platform to create a renderer and run:
   * Web step: `EmscriptenPlatform` + `Canvas2D`
   * Realtime: `SdlPlatform` + `WebGL`
5. Emit only:
   * `Frame.ui`
   * `Frame.plan`

Never call renderer APIs directly.

---

## Build targets

* Native builds use standard CMake.
* Web builds use **Emscripten** (`emcmake cmake`).

Do not add JS frameworks.
JS glue must be:

* minimal
* boring
* limited to loading wasm and forwarding input

---

## Build and commit workflow

### Native build (default)

```
cmake --build build
```

### Web build

```
emcmake cmake -S . -B build-web
cmake --build build-web
```

### Commit loop (for big tasks)

When asked to iterate per task item:

1. Change code.
2. Build (native: `cmake --build build`).
3. If build succeeds, commit with `codex: <short description>`.

Only use `git add .` and `git commit -am ""` unless explicitly told otherwise.

---

## Using third_party code

* `/third_party/edulcni` and `/third_party/shrooms` are **reference only**.
* Copy ideas and small utilities.
* Do **not** modify them directly.
* Do **not** re-introduce old architecture patterns (JS codegen, Scene-owned rendering).

---

## Style & constraints

* C++20
* No RTTI-heavy or macro-heavy frameworks
* Prefer explicit data structures over inheritance chains
* Keep interfaces small and concrete
* Avoid premature generalization
* run build before finishing your work
* when builds, commit with "codex: <short description>"

---

## If something feels unclear

Default to:

1. Emit data into `Frame`
2. Let the renderer consume it
3. Let drivers control timing
4. Let platform control I/O only

If in doubt, ask:
**"Is this retained state, or per-frame output?"**
If it is per-frame output, it belongs in `Frame`.

---

## Success criteria for this repo

This repo is successful if:

* A step-based algorithm visualization
* A real-time WebGL particle demo
  can coexist **without special cases** or duplicated infrastructure.

If an implementation breaks this, reconsider it.

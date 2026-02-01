# AetherEngine

**A Multiplayer-First, Data-Driven 2D Game Engine.**

> *"RPGMaker simplicity meets Unreal Engine networking."*

AetherEngine is a proprietary, source-closed game development suite designed to democratize the creation of authoritative multiplayer 2D RPGs. It provides a robust, "Black Box" environment where developers create games using data (Assets, Prefabs, Logic Graphs) rather than raw code, ensuring security and ease of distribution.

## Key Features

### Native Multiplayer Architecture
- **Authoritative Server:** The server is the single source of truth. Clients are "dumb" terminals that render state and send inputs.
- **Binary-Only Distribution:** Games are shipped as compiled binaries. Users never touch source code.
- **Integrated Database:** Every server build includes an embedded **SQLite** backend for persistent world data, user accounts, and inventory state.

### Architecture
- **Data-Oriented ECS:** Built on a custom Entity Component System for maximum cache coherence and performance.
- **Cross-Platform:** Native support for **Windows, Linux (Headless Server), and macOS**.
- **Partitioned Worlds:** Built-in chunk loading and interest management (NetCull) to support large persistent maps.
- **Deterministic Simulation:** Fixed-timestep logic updates decoupled from rendering frame rates.

### Technical Stack
- **Language:** C++20
- **Build System:** CMake + Ninja
- **Windowing:** SDL2 (Abstracted)
- **Scripting:** Node-Based Logic Graphs (Internal Intepreter)
- **Physics:** AABB Deterministic Collision (No non-deterministic physics engines)
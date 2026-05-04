# Ghost Ascent

> *"Discipline is Savior. Greed is Death."*

A vertical ascension platformer built with OpenGL. Play as a wandering spirit climbing the infinite tower, where every choice carries weight and the walls of consequence close in.

## The Philosophy

This is not just a platformer. It is a meditation on **discipline versus greed**:

- **Discipline** (White Dots): Patient collection pauses the crushing walls, buys time, and ultimately leads to salvation.
- **Greed** (Black Dots): The tempting shortcut pushes walls back significantly, but spawns new obstacles that make future ascents harder.

The puzzle is simple yet brutal: **You cannot collect every dot.** Choose your path wisely.

---

## How to Download and Run

### Download

```bash
git clone https://github.com/YOUR_USERNAME/Ghost-Ascent.git
cd Ghost-Ascent
```

### Build Requirements

- **CMake** 3.16 or higher
- **C++17** compatible compiler
- **OpenGL** 2.1+
- **GLFW3**
- **GLEW**

### Build Instructions

**Linux/macOS:**
```bash
mkdir build
cd build
cmake ..
cmake --build .
./GhostAscent
```

**Windows (Visual Studio 2022):**
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
# Run: .\Release\GhostAscent.exe
```

---

## Controls

| Key | Action |
|-----|--------|
| **A / ←** | Move left |
| **D / →** | Move right |
| **Space** | Jump (double-tap for double jump) |
| **W / ↑** | Look up |
| **S / ↓** | Look down |
| **R** | Restart (on game over) |
| **ESC** | Quit |

---

## Mechanics

### Core Systems

**1. The Closing Walls**
- Two walls constantly move inward from the screen edges
- Speed increases with height climbed
- Getting crushed = Game Over

**2. White Dots (Discipline)**
- Appear on platforms
- Collecting pauses walls for **2.4 seconds**
- Collecting 20 dots triggers the **Frozen Ascension** (victory)
- *Skipping a white dot increases wall speed by 35% (compounds up to 10x)*

**3. Black-Dot Platforms (Greed)**
- Spawn after every **10 white dots** collected
- Appear on screen edges (left or right)
- Contain **black dots** that push walls back **40%**
- BUT: They add permanent obstacles to your path
- The more you take, the harder the climb becomes

**4. The Frozen Ascension**
- Upon collecting the 20th white dot, time freezes
- The ghost levitates upward in a cinematic sequence
- A white void consumes the screen
- Victory: *"You Won"*

### Dynamic Difficulty

- **Height Stress**: Higher climbs = faster walls
- **Skip Stress**: Missing white dots = permanently faster walls (up to 10x multiplier)
- **Wall Pressure**: The minimum gap between walls shrinks as you climb

---

## The Puzzle: Discipline vs Greed

> **Hint:** The optimal path is not obvious. 
>
> Black dots give immediate relief (40% wall pushback) but spawn platforms that block your future path. White dots give temporary safety (2.4s pause) but progress toward the true win condition.
>
> **Can you resist the temptation of black dots and achieve pure discipline?**
> **Or will greed consume you, leaving you trapped between walls of your own making?**

The walls never truly stop. They only wait for your choice.

---

## Technical Details

See [TECHNICAL_REPORT.md](TECHNICAL_REPORT.md) for implementation details, architecture, and rendering pipeline.

---

## Project Structure

| File | Purpose |
|------|---------|
| `GameMain.cpp` | Entry point, main loop, state machine |
| `GhostController.cpp` | Player physics, double-jump, eye tracking |
| `LevelGenerator.cpp` | Procedural platforms, dot placement |
| `WallSystem.cpp` | Closing walls, collision detection |
| `BrickRenderer.cpp` | Procedural brick textures |
| `GameUi.cpp` | Lighting effects, text rendering, overlays |
| `BloodTexture.cpp` | Blood overlay on death |
| `MonitorManager.cpp` | Fullscreen/window management |

---

## Optional Assets

Place a `blood_drip.png` in the `assets/` folder for a textured blood overlay on game over. Without this file, the game uses a procedural blood effect.

---

## License

GNU General Public License v3.0 (GPL-3.0) - This software is free to use, modify, and distribute under the terms of the GPL. All derivative works must also be open source under GPL.

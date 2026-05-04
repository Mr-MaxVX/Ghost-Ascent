# Technical Report: Ghost Ascent

## Executive Summary

**Ghost Ascent** is a vertical ascension platformer built in C++17 using OpenGL 2.1 immediate mode rendering. The game features a custom physics engine, procedural level generation, dynamic difficulty scaling, and a philosophical win condition system centered around resource management (discipline vs greed).

--- 

## Architecture Overview

### Design Philosophy

The codebase follows a **component-based architecture** with clear separation of concerns:
- **Game Loop** (`GameMain.cpp`): State machine, timing, rendering orchestration
- **Player Controller** (`GhostController`): Physics, input, rendering
- **Level System** (`LevelGenerator`): Procedural generation, collision data
- **Threat System** (`WallSystem`): Dynamic difficulty, collision detection
- **Presentation Layer** (`GameUi`, `BrickRenderer`): Visual effects, UI

### State Machine

```
Intro_Idle → Intro_ZoomIn → Intro_Text → Intro_ZoomOut → Playing
                                                      ↓
                                            Win_Sequence (cinematic)
                                                      ↓
                                              Game_Won_Void
```

**Key States:**
- **Intro States**: Camera zooms from wide view to gameplay view with typewriter text
- **Playing**: Standard gameplay with wall closing and platform climbing
- **Win_Sequence**: Frozen camera, ghost levitation, white explosion overlay
- **Game_Won_Void**: Final victory screen with falling black feathers

---

## Core Systems

### 1. Ghost Controller (`GhostController.cpp`)

**Physics Implementation:**
```cpp
// Semi-implicit Euler integration
velocity += acceleration * dt;
position += velocity * dt;
```

**Features:**
- Double-jump with coyote time
- Variable jump height (based on spacebar hold)
- Eye tracking (follows vertical input or defaults to climb direction)
- Procedural sheet physics (simulated fabric movement)

**Collision Detection:**
- AABB (Axis-Aligned Bounding Box) against platform rectangles
- One-way platform support (can jump through from below)
- Wall collision handled separately by WallSystem

### 2. Wall System (`WallSystem.cpp`)

**Dynamic Difficulty Formula:**
```cpp
heightStress = max(0, maxHeightReached - 120.0f) * 0.018f;
speed = (baseSpeed + heightStress) * skipStress * bufferMultiplier;
```

| Factor | Value | Effect |
|--------|-------|--------|
| `baseSpeed` | 42.0f | Base closing speed |
| `heightStress` | Variable | +1.8% speed per 100px height above 120px |
| `skipStress` | 1.0x - 10.0x | +35% per skipped white dot (compounds) |
| `bufferMul` | 0.38x | Temporary slowdown on dot collection (2.4s) |

**Critical Design:** Walls are **unstoppable** - they never stop moving inward until they meet at the center. This creates constant pressure and prevents "safe camping."

**Collision:** Circle-AABB intersection test for ghost crushing.

### 3. Level Generator (`LevelGenerator.cpp`)

**Procedural Generation Algorithm:**
```
1. Divide screen into vertical bands (left, center-left, center, center-right, right)
2. Place initial platform at ground level
3. For each new platform:
   - Random x-position within valid band (respecting wall positions)
   - y-position = highest_platform + gap (80-160px)
   - Width: 60-120px, Height: 15-25px
4. Spawn white dot on ~70% of platforms
5. Every 10 dots: spawn black-dot platform on edge
```

**Black-Dot Platform Spawning:**
- Triggered when `blackDotSpawnCounter >= blackDotNextSpawnAt`
- First spawn at 10 white dots
- Subsequent spawns every 2-4 white dots
- **Win Condition Check:** Prevents spawn if `dotsCollectedTotal >= 20`

### 4. Rendering Pipeline (`GameUi.cpp`)

**OpenGL 2.1 Immediate Mode:**
All rendering uses `glBegin()/glEnd()` with vertex specification:
- `GL_QUADS`: Solid shapes (bricks, walls, ghost body)
- `GL_LINES`: Outlines, text rendering
- `GL_TRIANGLE_FAN`: Radial gradients (lighting effects)

**Visual Effects:**

| Effect | Technique |
|--------|-----------|
| **Radial Light** | Triangle fan with alpha gradient from center to edge |
| **Top Arc Fixture** | 150-segment arc with pulse animation, fades in at 10+ dots |
| **White Explosion** | Expanding circle (2000px/s) centered on ghost during Win_Sequence |
| **Blood Overlay** | Procedural rectangle generation OR textured quad with chroma key |
| **Typewriter Text** | Character-limited string drawing with timer-based reveal |

**Text Rendering:**
Custom vector font using `GL_LINES`:
- 5x7 grid per character
- Clean outline style with variable stroke weight
- Kerning and spacing support

### 5. Win Sequence Implementation

**Trigger:** `dotsCollectedTotal >= 20`

**Sequence Timeline:**
```
t=0.0s    → Freeze gameplay (skip updates)
t=0.0-2.2s → Ghost levitates at 45px/s
           → Camera frozen at capture position
           → Render normally (frozen state)
t=0.8s+   → White circle expands from ghost position
           → radius = (winTimer - 0.8) * 2000.0f
t=2.2s    → Transition to Game_Won_Void
           → White screen, falling feathers, "YOU WON"
```

**Technical Challenge:** The camera projection changes during Win_Sequence:
- **Before:** Normal camera following ghost
- **During:** Frozen camera (`winCamY` captured at trigger)
- **After:** UI projection (0,0 to fbW,fbH)

The white explosion must account for this projection change - it uses screen-center coordinates during Win_Sequence.

---

## Build System

**CMake Configuration:**
```cmake
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)

target_link_libraries(GhostAscent 
    OpenGL::GL
    GLEW::GLEW
    glfw
)
```

**Platform Support:**
- Windows (Visual Studio 2019/2022)
- Linux (GCC/Clang)
- macOS (requires framework linking for OpenGL)

---

## Performance Characteristics

| Metric | Value |
|--------|-------|
| **Target Framerate** | 60 FPS |
| **Draw Calls** | ~200-500 per frame |
| **Vertex Count** | ~2,000-5,000 per frame |
| **Memory Usage** | ~20-50 MB |
| **CPU Usage** | Single-threaded, minimal |

**Optimizations:**
- Immediate mode is acceptable due to low vertex count
- No texture streaming (procedural brick generation)
- Simple collision broad-phase (AABB rejection before narrow-phase)

---

## Known Issues & Solutions

### Issue: Camera Snap During Win_Sequence
**Cause:** `if (currentState != GameState::Playing)` caught Win_Sequence in intro zoom logic
**Solution:** Explicit state checking for Intro vs Win_Sequence vs Playing

### Issue: White Explosion Misaligned
**Cause:** Projection matrix changes during sequence
**Solution:** Use screen-center coordinates (fbW*0.5, fbH*0.5) instead of ghost world position

### Issue: Black Platform Spawn at Win
**Cause:** Dot collection triggered spawn logic before win check
**Solution:** Add `dotsCollectedTotal < 20` guard to spawn condition

---

## Future Enhancements

1. **Shader-Based Rendering**: Migrate from immediate mode to VBOs/VAOs with shaders
2. **Audio System**: Add music and SFX (currently silent)
3. **Save System**: Persist high scores and unlockables
4. **Mobile Port**: Touch controls, aspect ratio handling
5. **Modding Support**: Lua scripting for custom levels

---

## Credits & Dependencies

| Dependency | Purpose | License |
|------------|---------|---------|
| GLFW3 | Window/Input management | zlib/png |
| GLEW | OpenGL extension loading | BSD/MIT |
| stb_image | Image loading (optional) | Public Domain |
| CMake | Build system | BSD |

---

## Conclusion

**Ghost Ascent** demonstrates that compelling gameplay emerges from simple systems with meaningful choices. The technical implementation prioritizes clarity and immediate feedback over complex architecture. The game's true depth lies not in its rendering pipeline, but in the philosophical tension encoded in its mechanics: the player must choose between short-term relief (greed/black dots) and long-term victory (discipline/white dots).

The codebase serves as a reference for:
- OpenGL 2.1 immediate mode techniques
- State machine implementation in games
- Procedural content generation
- Dynamic difficulty adjustment
- Cinematic sequence programming

*"Discipline is Savior. Greed is Death."*

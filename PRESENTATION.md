# Ghost Ascent - Technical Presentation

> **Discipline is Savior. Greed is Death.**

---

# Slide 1: Title Slide

## Ghost Ascent
### A Vertical Ascension Platformer Built with OpenGL

**[PLACE TITLE SCREENSHOT HERE]**

*Built in C++17 | OpenGL 2.1 | Custom Physics Engine*

---

# Slide 2: Game Overview

## What is Ghost Ascent?

- **Genre:** Vertical platformer with dynamic difficulty
- **Core Mechanic:** Climb while managing closing walls
- **Philosophy:** Choice between Discipline (white dots) and Greed (black dots)
- **Win Condition:** Collect 20 white dots → Frozen Ascension cinematic

**[PLACE GAMEPLAY SCREENSHOT HERE - SHOWING WALLS AND GHOST]**

---

# Slide 3: Architecture Overview

## Component-Based Design

```
┌─────────────────────────────────────┐
│         GameMain.cpp                │
│      (State Machine, Loop)          │
└──────────────┬──────────────────────┘
               │
    ┌──────────┼──────────┬──────────┐
    │          │          │          │
┌───▼───┐  ┌──▼────┐ ┌───▼───┐ ┌──▼────┐
│ Ghost │  │ Level │ │ Walls │ │  UI   │
│Control│  │Generator│ │System │ │Render │
└───────┘  └───────┘ └───────┘ └───────┘
```

**[PLACE ARCHITECTURE DIAGRAM HERE]**

| Module | File | Responsibility |
|--------|------|--------------|
| Game Loop | `GameMain.cpp` | States, timing, rendering |
| Player | `GhostController.cpp` | Physics, input, double-jump |
| Level | `LevelGenerator.cpp` | Procedural platforms, dots |
| Threats | `WallSystem.cpp` | Dynamic difficulty, collision |
| Visuals | `GameUi.cpp` | Lighting, text, overlays |

---

# Slide 4: State Machine

## Game States Flow

```
Intro_Idle → Intro_ZoomIn → Intro_Text → Intro_ZoomOut
                                                        ↓
                                                 Playing
                                                    ↓
                            ┌───────────────────────┴───────────────────────┐
                            │                                               │
                    Game Over (blood effect)                          Win_Sequence
                            │                                               │
                            ↓                                               ↓
                         Restart                                    Game_Won_Void
```

**[PLACE STATE DIAGRAM SCREENSHOT HERE]**

### Key States:
- **Intro**: Camera zoom + typewriter text reveal
- **Playing**: Normal gameplay with wall closing
- **Win_Sequence**: Frozen camera, ghost levitation, white explosion
- **Game_Won_Void**: White screen, falling black feathers

---

# Slide 5: Ghost Controller - Physics

## Player Physics Implementation

```cpp
// Semi-implicit Euler integration
velocity += acceleration * dt;
position += velocity * dt;
```

### Features:
- **Double Jump**: Coyote time + air jump
- **Variable Jump Height**: Based on spacebar hold duration
- **Eye Tracking**: Follows vertical input or climb direction
- **Collision**: AABB vs platform rectangles

**[PLACE GHOST CODE SNIPPET SCREENSHOT HERE]**

### Input Handling:
| Key | Action |
|-----|--------|
| A/D or Arrows | Horizontal movement |
| Space | Jump (double-tap for double jump) |
| W/S or Up/Down | Eye direction |

---

# Slide 6: Wall System - Dynamic Difficulty

## The Closing Walls

### Speed Formula:
```cpp
heightStress = max(0, maxHeightReached - 120.0f) * 0.018f;
speed = (baseSpeed + heightStress) * skipStress * bufferMul;
```

**[PLACE WALL SPEED CHART/DIAGRAM HERE]**

### Difficulty Factors:

| Factor | Base | Effect |
|--------|------|--------|
| `baseSpeed` | 42.0f | Constant base rate |
| `heightStress` | Variable | +1.8% per 100px above 120px |
| `skipStress` | 1.0x - 10.0x | +35% per skipped dot |
| `bufferMul` | 0.38x | 2.4s slowdown on dot collect |

### Design Choice: Unstoppable Walls
- Walls NEVER stop moving inward
- They only pause briefly on dot collection
- Creates constant pressure - no safe camping

**[PLACE WALL CLOSING SCREENSHOT HERE]**

---

# Slide 7: Level Generation

## Procedural Platform Algorithm

```cpp
// Pseudocode
for each new platform:
    x = random_band_position(respecting wall positions)
    y = highest_platform + random_gap(80-160px)
    width  = random(60-120px)
    height = random(15-25px)
    
    if random() < 0.7:
        spawn_white_dot_on_platform()
```

**[PLACE PLATFORM GENERATION SCREENSHOT HERE]**

### Spawn Rules:
- **White Dots**: ~70% of platforms
- **Black-Dot Platforms**: Every 10 white dots collected → edge spawn
- **Black Dots**: Push walls back 40% (but add obstacles)

---

# Slide 8: The Core Puzzle - Discipline vs Greed

## Two Resources, One Choice

**[PLACE SPLIT SCREEN IMAGE: WHITE DOT vs BLACK DOT]**

### White Dots (Discipline):
- ✅ Pause walls for 2.4s
- ✅ Progress toward win (20 dots)
- ❌ No immediate wall relief
- **Philosophy:** Patience leads to salvation

### Black Dots (Greed):
- ✅ Immediate 40% wall pushback
- ✅ More breathing room
- ❌ Spawn blocking platforms permanently
- ❌ No progress toward win condition
- **Philosophy:** Shortcuts create future obstacles

> **The Puzzle:** You cannot collect everything. Every choice matters.

---

# Slide 9: Win Condition - Frozen Ascension

## Cinematic Sequence Timeline

**[PLACE WIN SEQUENCE SCREENSHOTS - 3 PANELS]**

| Time | Event |
|------|-------|
| t=0.0s | Trigger: 20th dot collected. Gameplay freezes. |
| t=0.0-2.2s | Ghost levitates at 45px/s. Camera frozen. |
| t=0.8s+ | White circle expands from ghost: `radius = (t-0.8) * 2000` |
| t=2.2s | White void consumes screen. Transition to victory. |

### Technical Implementation:
- **Camera Freeze:** Capture `winCamY` at trigger, hold until sequence ends
- **Ghost Levitation:** `ghost.y = winStartY + (winTimer * 45.0f)`
- **White Explosion:** `GL_TRIANGLE_FAN` with expanding radius
- **State Isolation:** Skip all gameplay updates during sequence

---

# Slide 10: Rendering Pipeline

## OpenGL 2.1 Immediate Mode

**[PLACE RENDERING COMPARISON IMAGE - WIREFRAME vs FINAL]**

### Rendering Techniques:

| Primitive | Usage |
|-----------|-------|
| `GL_QUADS` | Solid shapes (ghost, bricks, walls) |
| `GL_LINES` | Outlines, text rendering, ghost face |
| `GL_TRIANGLE_FAN` | Radial gradients (lighting, explosion) |

### Visual Effects:

| Effect | Implementation |
|--------|---------------|
| **Radial Light** | Triangle fan + alpha gradient |
| **Top Arc** | 150-segment arc with pulse animation |
| **White Explosion** | Expanding circle (2000px/s) |
| **Blood Overlay** | Procedural rectangles OR textured quad |
| **Typewriter Text** | Char-limited draw with timer reveal |

---

# Slide 11: Custom Text Rendering

## Vector Font System

**[PLACE TEXT RENDERING COMPARISON IMAGE]**

### No External Font Files!
- Custom 5x7 grid per character
- Drawn with `GL_LINES` for clean outlines
- Supports variable stroke weight
- Full kerning and spacing

### Used For:
- "GAME OVER" text
- "YOU WON" victory text
- "DISCIPLINE IS SAVIOR / GREED IS DEATH" intro
- All UI text (no dependencies!)

---

# Slide 12: Build System

## CMake Cross-Platform

```cmake
project(GhostAscent VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(GhostAscent 
    OpenGL::GL GLEW::GLEW glfw)
```

### Build Commands:

**Linux/macOS:**
```bash
mkdir build && cd build
cmake .. && cmake --build .
./GhostAscent
```

**Windows:**
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
.\Release\GhostAscent.exe
```

**[PLACE BUILD OUTPUT SCREENSHOT HERE]**

---

# Slide 13: Key Technical Challenges & Solutions

## Bugs Fixed During Development

| Issue | Cause | Solution |
|-------|-------|----------|
| Camera snap in Win_Sequence | `!= Playing` caught Win_Sequence too | Explicit state checks |
| White explosion misaligned | Projection matrix changed mid-sequence | Use screen-center coords |
| Black platform at win | Spawn logic triggered before win check | Add `dotsCollectedTotal < 20` guard |
| Text upside down | Wrong Y coordinate system | Flip Y for screen space (0,0 at bottom) |
| Wall stopping early | `centerOffset` + `minHalfGap` interaction | Remove offset, true center clamp |

**[PLACE BEFORE/AFTER COMPARISON SCREENSHOTS HERE]**

---

# Slide 14: Performance

## Lightweight by Design

| Metric | Value |
|--------|-------|
| **Target FPS** | 60 |
| **Draw Calls** | 200-500/frame |
| **Vertices** | 2,000-5,000/frame |
| **Memory** | ~20-50 MB |
| **CPU** | Single-threaded |

### Why OpenGL 2.1?
- Low vertex count makes immediate mode acceptable
- No need for complex shader pipeline
- Procedural generation = no texture streaming
- Maximum compatibility (old hardware support)

**[PLACE PROFILER/STATS SCREENSHOT HERE]**

---

# Slide 15: Future Roadmap

## Potential Enhancements

**[PLACE CONCEPT ART OR MOCKUP HERE]**

- **Shader Migration:** VBOs/VAOs + GLSL shaders
- **Audio System:** Music and SFX (currently silent)
- **Save System:** High scores, progress persistence
- **Mobile Port:** Touch controls, adaptive aspect ratio
- **Modding Support:** Lua scripting for custom levels
- **Multiplayer:** Race mode (shared seed, compare times)

---

# Slide 16: Conclusion

## What We Built

**[PLACE FINAL VICTORY SCREENSHOT HERE]**

### Ghost Ascent is:
- A complete vertical platformer in **~1,500 lines of C++**
- A study in **meaningful choice** (discipline vs greed)
- A showcase of **OpenGL 2.1** techniques
- A demonstration of **procedural generation**
- A lesson in **dynamic difficulty** design

> *"The walls never truly stop. They only wait for your choice."*

---

# Slide 17: Live Demo / Q&A

## Try It Yourself

```bash
git clone https://github.com/Mr-MaxVX/Ghost-Ascent.git
cd Ghost-Ascent
mkdir build && cd build
cmake .. && cmake --build .
./GhostAscent
```

**[PLACE QR CODE OR GITHUB LINK SCREENSHOT HERE]**

## Questions?

- Why OpenGL 2.1 instead of modern Vulkan/DirectX?
- How does the difficulty curve feel over 20 dots?
- What other philosophical themes could be encoded?
- How would you balance the black/white dot ratio?

---

## Image Placeholder Guide

**[PLACE SCREENSHOT 1]:** Title screen / main menu
**[PLACE SCREENSHOT 2]:** Early gameplay (ground level, walls visible)
**[PLACE SCREENSHOT 3]:** Mid-game (higher up, platforms spread out)
**[PLACE SCREENSHOT 4]:** Collecting a white dot (buffer effect)
**[PLACE SCREENSHOT 5]:** Black-dot platform on screen edge
**[PLACE SCREENSHOT 6]:** Game Over screen with blood overlay
**[PLACE SCREENSHOT 7]:** Win sequence - ghost levitating
**[PLACE SCREENSHOT 8]:** White explosion consuming screen
**[PLACE SCREENSHOT 9]:** Victory screen "YOU WON"
**[PLACE SCREENSHOT 10]:** Code snippet - wall system or physics

---

## Notes for Presenter

- **Demo tip:** Collect 3-4 white dots quickly, then show wall pause
- **Key talking point:** The unstoppable walls create tension - no safe spots
- **Philosophy hook:** Ask audience - "Would you take the black dot?"
- **Technical highlight:** Everything is procedural - no asset files needed
- **Build time:** Mention CMake builds in ~10 seconds

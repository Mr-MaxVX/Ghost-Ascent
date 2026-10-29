# Ghost Ascent — Technical Notes

This is a breakdown of how the game works under the hood. I built this as a learning project in OpenGL and C++, so some choices are intentionally simple rather than "production-grade."

---

## What This Is

A vertical platformer where you play as a sheet ghost climbing an endless tower. Two walls close in from the sides, and you manage their speed by collecting dots — white ones slow the walls temporarily, black ones push them back permanently but spawn obstacles. Collect 20 white dots and you win via a short cinematic sequence.

Built with: C++17, OpenGL 2.1, GLFW3, GLEW, CMake.

---

## How the Code is Organized

I split things into separate files roughly by responsibility:

- `GameMain.cpp` — the main loop, state machine, and all the rendering setup
- `GhostController.cpp` — player movement, jump physics, drawing the ghost
- `LevelGenerator.cpp` — makes platforms and places dots as you climb
- `WallSystem.cpp` — moves the walls, handles difficulty, crushes the player
- `BrickRenderer.cpp` — draws the brick pattern on platforms
- `GameUi.cpp` — text, lighting effects, blood overlay, win screen

Each `.cpp` has a matching `.hpp` with declarations so other files can include them.

---

## The State Machine

The game moves through a few distinct states:

```
Intro_Idle → Intro_ZoomIn → Intro_Text → Intro_ZoomOut → Playing
                                                              ↓
                                                    Win_Sequence (cinematic)
                                                              ↓
                                                        Game_Won_Void
```

**Intro:** The camera zooms in on the ghost, text types out "DISCIPLINE IS SAVIOR / GREED IS DEATH", then zooms back to normal gameplay view.

**Playing:** Standard platforming. Walls close in, you jump and collect dots.

**Win_Sequence:** Time freezes. The ghost floats upward. A white circle expands from the ghost to fill the screen. After 2.2 seconds it transitions to the white void victory screen.

The intro and win sequence were added late in development — the state machine made it easy to bolt them on without breaking the main loop.

---

## Physics

I used semi-implicit Euler integration because it's dead simple and stable enough for this:

```cpp
velocity += acceleration * dt;
position += velocity * dt;
```

Gravity pulls down, jump sets a negative vertical velocity, and that's basically it. There's a double-jump with a small coyote time window so you don't have to frame-perfect your inputs.

Collision is AABB vs rectangles for platforms. One-way platforms let you jump up through them from below. Wall crushing is a circle-AABB check against the blade tips on each wall.

---

## The Walls (The Core Tension)

This is where most of the design effort went. The walls constantly move inward. Their speed is:

```cpp
speed = (baseSpeed + heightStress) * skipStress * bufferMul;
```

- `baseSpeed` = 42 px/s. Starting point.
- `heightStress` grows as you climb higher — past 120px height it starts adding pressure.
- `skipStress` starts at 1x. Every time you skip a white dot (pass under it without collecting), it multiplies by 1.35x, capped at 10x. This adds up fast.
- `bufferMul` drops to 0.38x for 2.4 seconds when you collect a white dot — that's your breathing room.

The walls never stop. I intentionally removed any "minimum gap" logic because I wanted the player to feel constant pressure. They only pause briefly when you collect a white dot.

---

## Dots and the Discipline/Greed Choice

**White dots** appear on most platforms (~70%). Collecting one:
- Triggers the 2.4s wall slow (buffer)
- Slightly reduces your skipStress (forgiveness mechanic)
- Counts toward the 20 needed to win

**Black-dot platforms** spawn after every 10 white dots collected. They appear on the left or right edge of the screen and contain black dots. Collecting a black dot:
- Pushes both walls back by 40% (opens up space)
- Slows current wall speed by 45%
- BUT spawns a permanent obstacle platform that blocks future paths

The idea is white dots = discipline (patient, incremental, leads to win). Black dots = greed (instant relief, but you pay for it later with harder traversal). You can ignore black dots entirely and just focus on white ones, or you can grab them when panicking — your choice.

---

## Rendering

I used OpenGL 2.1 immediate mode (`glBegin`/`glEnd`). Not modern, not fast, but perfectly fine for a 2D game with only a few thousand vertices per frame. No shaders needed.

- `GL_QUADS` — solid shapes (ghost body, bricks, walls, platforms)
- `GL_LINES` — outlines, text rendering, the ghost's face
- `GL_TRIANGLE_FAN` — radial gradients for lighting effects

### Custom Text

I drew my own font rather than using a library. Each character is a 5x7 grid rendered with `GL_LINES`. It looks clean at any zoom level and has zero dependencies. The intro text uses a typewriter effect that reveals characters over time.

### Lighting

There's a radial light around the ghost (only appears after the first black dot spawns) and a pulsing arc at the top of the screen that fades in after 10 dots. Both use alpha gradients via `GL_TRIANGLE_FAN`.

---

## The Win Sequence

When `dotsCollectedTotal >= 20`, the game enters `Win_Sequence`:

1. Capture the camera Y position and freeze it
2. Skip all gameplay updates (ghost doesn't fall, walls don't move)
3. Ghost levitates upward at 45 px/s
4. After 0.8 seconds, draw a white circle expanding from the ghost at 2000 px/s
5. At 2.2 seconds, switch to `Game_Won_Void` — white screen, falling black feathers, "YOU WON" text

The tricky part was the camera. During the intro I use zoom-based projection centered on the ghost. During gameplay I use a normal scrolling camera. During the win sequence I need the frozen camera. Originally I had `if (currentState != Playing)` to handle the intro zoom, which accidentally caught the win sequence too and caused a violent camera snap. Fixed it by explicitly checking each state.

---

## Build Setup

CMake handles everything. On Windows with Visual Studio:

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

Dependencies: OpenGL, GLEW, GLFW3. stb_image is bundled as a single header.

---

## Bugs were Fixed

**Camera snap during win** — The intro zoom logic used `if (state != Playing)` which also caught the win sequence. Split it into explicit state checks.

**White explosion misaligned** — I was calculating the explosion position using world coordinates, but during Win_Sequence the projection matrix is different. Switched to screen-center coordinates.

**Black platform spawning at win** — Collecting the 20th white dot would trigger both the win state AND the black platform spawn logic. Added a `dotsCollectedTotal < 20` guard.

**Text upside down** — Initially flipped Y because OpenGL puts (0,0) at bottom-left but I was thinking in top-left screen coordinates.

---

## Performance

This thing is lightweight. Runs at 60 FPS on basically anything.

- Draw calls: a few hundred per frame
- Vertices: 2,000–5,000 per frame
- Memory: ~30 MB
- Entirely single-threaded

Immediate mode is "bad" for modern GPUs, but when your vertex count is this low it genuinely does not matter. The bottleneck is never the GPU.

---

## What might be done differently

- **Audio:** There's no sound. At all. I didn't want to deal with audio libraries for this project but it definitely needs some.
- **Modern OpenGL:** VBOs and a simple shader would be cleaner than immediate mode, even if performance isn't an issue.
- **Save data:** No persistence. High scores, best times, nothing is saved.
- **More juice:** Screen shake on wall crush, better particle effects, more visual feedback on dot collection.

---

## Dependencies

| Library | What it does | License |
|---------|-------------|---------|
| GLFW3 | Window creation, input handling | zlib/png |
| GLEW | OpenGL extension loading | BSD/MIT |
| stb_image | Load PNGs (optional blood texture) | Public Domain |

---

*"Discipline is Savior. Greed is Death."*

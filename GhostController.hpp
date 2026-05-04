#pragma once

struct GLFWwindow;

struct AABB {
    float x, y, w, h;
};

struct GhostController {
    float x = 400.0f;
    float y = 180.0f;
    float vx = 0.0f;
    float vy = 0.0f;
    float radius = 22.0f;
    bool onGround = false;
    bool jumpHeld = false;
    int jumpsUsed = 0; // 0..2: double-jump (two presses before landing)
    // Soul-gaze: smoothed pupil offset (world units); idle looks “forward” at the player.
    float eyeLookX = 0.0f;
    float eyeLookY = 0.0f;

    void Reset(float spawnX, float spawnY);

    // Smooth, floaty platformer feel; dt in seconds.
    void Update(GLFWwindow* window, float dt, float worldWidth, float groundTopY, const AABB* platforms, int platformCount);
    void Draw() const;
};

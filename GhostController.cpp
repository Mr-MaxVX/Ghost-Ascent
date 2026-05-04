#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "GhostController.hpp"
#include <cmath>
#include <algorithm>

void GhostController::Reset(float spawnX, float spawnY) {
    x = spawnX;
    y = spawnY;
    vx = vy = 0.0f;
    onGround = false;
    jumpHeld = false;
    jumpsUsed = 0;
    eyeLookX = eyeLookY = 0.0f;
}

static bool circleIntersectsAABB(float cx, float cy, float r, const AABB& b) {
    float nx = std::max(b.x, std::min(cx, b.x + b.w));
    float ny = std::max(b.y, std::min(cy, b.y + b.h));
    float dx = cx - nx;
    float dy = cy - ny;
    return (dx * dx + dy * dy) < r * r;
}

void GhostController::Update(GLFWwindow* window, float dt, float worldWidth, float groundTopY,
                             const AABB* platforms, int platformCount) {
    const float gravity = 520.0f;
    const float maxFall = 640.0f;
    const float moveAccel = 1400.0f;
    const float maxRun = 260.0f;
    const float jumpVel = 360.0f;
    const float airControl = 0.55f;
    const float groundFriction = 10.0f;

    float input = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) input -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) input += 1.0f;

    float accel = moveAccel * (onGround ? 1.0f : airControl);
    vx += input * accel * dt;
    float fric = std::exp(-groundFriction * dt);
    if (onGround && input == 0.0f) vx *= fric;
    if (vx > maxRun) vx = maxRun;
    if (vx < -maxRun) vx = -maxRun;

    bool wantJump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    bool jumpEdge = wantJump && !jumpHeld;
    jumpHeld = wantJump;
    if (jumpEdge && jumpsUsed < 2) {
        vy = jumpVel * (jumpsUsed == 0 ? 1.0f : 0.88f);
        jumpsUsed++;
        onGround = false;
    }

    vy -= gravity * dt;
    if (vy < -maxFall) vy = -maxFall;

    x += vx * dt;
    y += vy * dt;

    float minX = radius + 2.0f;
    float maxX = worldWidth - radius - 2.0f;
    if (x < minX) { x = minX; vx = 0.0f; }
    if (x > maxX) { x = maxX; vx = 0.0f; }

    onGround = false;

    // Ground slab
    if (y - radius < groundTopY) {
        y = groundTopY + radius;
        if (vy < 0.0f) vy = 0.0f;
        onGround = true;
        jumpsUsed = 0;
    }

    // Platforms (resolve from below)
    for (int i = 0; i < platformCount; ++i) {
        const AABB& p = platforms[i];
        if (!circleIntersectsAABB(x, y, radius * 0.92f, p)) continue;
        // Landing on top
        if (vy <= 0.0f && y > p.y + p.h * 0.5f) {
            float top = p.y + p.h;
            if (y - radius < top + 6.0f && x + radius * 0.7f > p.x && x - radius * 0.7f < p.x + p.w) {
                y = top + radius;
                vy = 0.0f;
                onGround = true;
                jumpsUsed = 0;
            }
        }
    }

    // Eye look: WASD → offset up to maxLook; idle → gentle forward / soul-gaze (+Y).
    float lx = 0.0f, ly = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) lx -= 1.0f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) lx += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) ly += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) ly -= 1.0f;

    const float maxLook = 0.1f * radius;
    float tgx, tgy;
    if (lx == 0.0f && ly == 0.0f) {
        tgx = 0.0f;
        tgy = maxLook * 0.95f;
    } else {
        float len = std::sqrt(lx * lx + ly * ly);
        tgx = (lx / len) * maxLook;
        tgy = (ly / len) * maxLook;
    }
    const float tau = 0.2f;
    float k = 1.0f - std::exp(-dt / tau);
    eyeLookX += (tgx - eyeLookX) * k;
    eyeLookY += (tgy - eyeLookY) * k;
}

void GhostController::Draw() const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const float r = radius;
    // Pokémon-style sheet ghost: round head, tapered sides, scalloped tail (symmetric outline)
    static const float ox[] = {
        0.00f,  0.22f,  0.42f,  0.58f,  0.70f,  0.76f,  0.74f,  0.62f,  0.48f,  0.30f,
        0.14f,  0.00f, -0.14f, -0.30f, -0.48f, -0.62f, -0.74f, -0.76f, -0.70f, -0.58f,
        -0.42f, -0.22f,
    };
    static const float oy[] = {
        0.88f,  0.82f,  0.68f,  0.48f,  0.22f, -0.05f, -0.28f, -0.48f, -0.62f, -0.70f,
        -0.66f, -0.58f, -0.66f, -0.70f, -0.62f, -0.48f, -0.28f, -0.05f,  0.22f,  0.48f,
        0.68f,  0.82f,
    };
    const int nv = (int)(sizeof(ox) / sizeof(ox[0]));
    const float cx = x;
    const float cy = y + r * 0.06f;

    glColor4f(0.62f, 0.62f, 0.66f, 0.48f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i < nv; ++i)
        glVertex2f(x + ox[i] * r, y + oy[i] * r);
    glVertex2f(x + ox[0] * r, y + oy[0] * r);
    glEnd();

    const float hi = 0.78f;
    glColor4f(0.78f, 0.78f, 0.82f, 0.28f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy + r * 0.08f);
    for (int i = 0; i < nv; ++i)
        glVertex2f(x + ox[i] * r * hi, y + oy[i] * r * hi + r * 0.06f);
    glVertex2f(x + ox[0] * r * hi, y + oy[0] * r * hi + r * 0.06f);
    glEnd();

    // Cool fold / painterly depth (sheet ghost)
    glColor4f(0.38f, 0.42f, 0.52f, 0.18f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy - r * 0.15f);
    for (int i = 0; i < nv; ++i) {
        float t = (oy[i] < 0.15f) ? 0.88f : 0.72f;
        glVertex2f(x + ox[i] * r * t, y + oy[i] * r * t);
    }
    glVertex2f(x + ox[0] * r * 0.88f, y + oy[0] * r * 0.88f);
    glEnd();

    const float exo = eyeLookX;
    const float eyo = eyeLookY;
    // Deep void eyes (soul-gaze), offset by smoothed look
    auto eye = [&](float ebx, float eby) {
        float ex = ebx + exo;
        float ey = eby + eyo;
        glColor4f(0.04f, 0.05f, 0.08f, 0.92f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(ex, ey);
        const int ne = 14;
        for (int i = 0; i <= ne; ++i) {
            float a = (6.2831853f * i) / ne;
            glVertex2f(ex + std::cos(a) * r * 0.13f, ey + std::sin(a) * r * 0.17f);
        }
        glEnd();
    };
    eye(x + r * 0.26f, y + r * 0.38f);
    eye(x - r * 0.26f, y + r * 0.38f);

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

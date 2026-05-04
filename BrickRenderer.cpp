#include "BrickRenderer.hpp"
#include <GL/glew.h>
#include <algorithm>
#include <cmath>

namespace {

inline void setColor(float r, float g, float b, float a = 1.0f) {
    glColor4f(r, g, b, a);
}

// Stable pseudo-random in [0,1) from grid cell + salt.
inline float cellHash(int ix, int iy, unsigned salt) {
    unsigned v = (unsigned)(ix * 374761393u + iy * 668265263u + salt * 1442695041u);
    v = (v ^ (v >> 13)) * 1274126177u;
    return (v & 0xFFFFFFu) / 16777216.0f;
}

inline void rect(float x0, float y0, float x1, float y1) {
    glVertex2f(x0, y0);
    glVertex2f(x1, y0);
    glVertex2f(x1, y1);
    glVertex2f(x0, y1);
}

} // namespace

void DrawDetailedBrickRect(float x, float y, float w, float h, unsigned salt) {
    const float mortar = 3.0f;
    const float rowH = 22.0f;
    const float baseBrickW = 52.0f;

    if (w <= 0.0f || h <= 0.0f) return;

    // Mortar backing (darker)
    setColor(0.12f, 0.10f, 0.09f, 1.0f);
    glBegin(GL_QUADS);
    rect(x, y, x + w, y + h);
    glEnd();

    int row0 = (int)std::floor(y / rowH) - 1;
    int row1 = (int)std::ceil((y + h) / rowH) + 1;

    for (int row = row0; row <= row1; ++row) {
        float y0m = row * rowH + mortar * 0.5f;
        float y1m = (row + 1) * rowH - mortar * 0.5f;
        float clipY0 = std::max(y, y0m);
        float clipY1 = std::min(y + h, y1m);
        if (clipY1 <= clipY0) continue;

        float stagger = ((row & 1) ? baseBrickW * 0.5f : 0.0f);
        float colStart = std::floor((x - stagger) / baseBrickW) * baseBrickW + stagger;

        for (float bx = colStart; bx < x + w + baseBrickW; bx += baseBrickW) {
            float x0m = bx + mortar * 0.5f;
            float x1m = bx + baseBrickW - mortar * 0.5f;
            float clipX0 = std::max(x, x0m);
            float clipX1 = std::min(x + w, x1m);
            if (clipX1 <= clipX0) continue;

            int ix = (int)std::floor(bx / baseBrickW);
            float t = cellHash(ix, row, salt);
            float age = cellHash(ix + 17, row - 3, salt);
            // Dark brown brick body with variation
            float r = 0.22f + 0.08f * t - 0.04f * age;
            float g = 0.10f + 0.04f * t;
            float b = 0.06f + 0.03f * (1.0f - t);
            setColor(r, g, b, 1.0f);
            glBegin(GL_QUADS);
            rect(clipX0, clipY0, clipX1, clipY1);
            glEnd();

            // Subtle edge chips (darker slivers)
            if (cellHash(ix, row, salt + 91u) > 0.72f) {
                setColor(r * 0.55f, g * 0.55f, b * 0.55f, 1.0f);
                float chipW = (clipX1 - clipX0) * 0.08f;
                glBegin(GL_QUADS);
                rect(clipX0, clipY0, clipX0 + chipW, clipY1);
                glEnd();
            }
        }
    }
}

void DrawShineDot(float cx, float cy, float baseRadius, float timeSec) {
    const float pulse = 0.85f + 0.15f * std::sin(timeSec * 6.0f);
    float r = baseRadius * pulse;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Outer bloom
    setColor(1.0f, 1.0f, 1.0f, 0.12f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    const int n = 24;
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        glVertex2f(cx + std::cos(a) * r * 2.2f, cy + std::sin(a) * r * 2.2f);
    }
    glEnd();

    setColor(1.0f, 1.0f, 1.0f, 0.35f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        glVertex2f(cx + std::cos(a) * r * 1.35f, cy + std::sin(a) * r * 1.35f);
    }
    glEnd();

    // Core
    setColor(1.0f, 1.0f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        glVertex2f(cx + std::cos(a) * r * 0.55f, cy + std::sin(a) * r * 0.55f);
    }
    glEnd();

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawBlackReliefDot(float cx, float cy, float baseRadius, float timeSec) {
    const float pulse = 0.88f + 0.12f * std::sin(timeSec * 4.5f);
    float r = baseRadius * pulse;
    const float borderOut = r + std::max(3.5f, baseRadius * 0.42f);
    const float borderIn = r + std::max(1.2f, baseRadius * 0.12f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const int n = 28;
    // Crisp white outer ring (“boarder”)
    setColor(0.98f, 0.98f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        float c = std::cos(a), s = std::sin(a);
        glVertex2f(cx + c * borderIn, cy + s * borderIn);
        glVertex2f(cx + c * borderOut, cy + s * borderOut);
    }
    glEnd();

    setColor(0.04f, 0.04f, 0.06f, 0.98f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        glVertex2f(cx + std::cos(a) * r, cy + std::sin(a) * r);
    }
    glEnd();

    setColor(0.85f, 0.85f, 0.9f, 0.55f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        float c = std::cos(a), s = std::sin(a);
        glVertex2f(cx + c * r * 0.92f, cy + s * r * 0.92f);
        glVertex2f(cx + c * r * 0.99f, cy + s * r * 0.99f);
    }
    glEnd();

    setColor(0.92f, 0.92f, 0.96f, 0.45f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx - r * 0.22f, cy + r * 0.22f);
    for (int i = 0; i <= n; ++i) {
        float a = (6.2831853f * i) / n;
        glVertex2f(cx - r * 0.22f + std::cos(a) * r * 0.18f, cy + r * 0.22f + std::sin(a) * r * 0.18f);
    }
    glEnd();

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

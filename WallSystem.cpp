#include <GL/glew.h>
#include "WallSystem.hpp"
#include <cmath>
#include <algorithm>

void WallSystem::Reset(float screenW) {
    leftX = 0.0f;
    rightX = screenW;
    currentSpeed = baseSpeed;
    skipStress = 1.0f;
    heightStress = 0.0f;
    bufferTimer = 0.0f;
}

void WallSystem::OnDotCollected() {
    bufferTimer = kBufferDuration;
    skipStress = std::max(1.0f, skipStress * 0.92f);
}

void WallSystem::OnDotSkipped() {
    // Aggressive penalty: 35% speed increase per skip, up to 10x
    skipStress = std::min(10.0f, skipStress * 1.35f);
}

void WallSystem::ApplyBlackDotWallRelief(float screenW) {
    const float kKeep = 0.6f; // 40% reduction in how far walls have closed from edges
    leftX *= kKeep;
    rightX = screenW - (screenW - rightX) * kKeep;
    if (leftX < 0.0f) leftX = 0.0f;
    if (rightX > screenW) rightX = screenW;
    // Also slow down walls by 45%
    currentSpeed *= 0.55f;
}

void WallSystem::Update(float dt, float screenW, GhostController& ghost, LevelGenerator& level) {
    // Height pressure: higher max altitude => faster closure
    float h = std::max(0.0f, level.maxHeightReached - 120.0f);
    heightStress = h * 0.018f;

    if (bufferTimer > 0.0f) bufferTimer -= dt;

    float bufferMul = (bufferTimer > 0.0f) ? kBufferSpeedMul : 1.0f;
    float targetSpeed = (baseSpeed + heightStress) * skipStress * bufferMul;
    // Smooth speed changes (floaty game feel for the "timer")
    currentSpeed += (targetSpeed - currentSpeed) * std::min(1.0f, dt * 2.5f);

    // Unstoppable walls: always move inward until they meet at center
    leftX += currentSpeed * dt;
    rightX -= currentSpeed * dt;
    
    // Clamp to true center - walls never stop until they reach middle
    if (leftX > screenW * 0.5f) leftX = screenW * 0.5f;
    if (rightX < screenW * 0.5f) rightX = screenW * 0.5f;
    
    // Final safety: ensure right never crosses left
    if (rightX < leftX) {
        float mid = (leftX + rightX) * 0.5f;
        leftX = mid;
        rightX = mid;
    }
    
    // Ensure we never go past screen edges
    if (leftX < 0.0f) leftX = 0.0f;
    if (rightX > screenW) rightX = screenW;

    float screenGhostX = ghost.x;

    for (auto& p : level.platforms) {
        if (!p.hasDot || p.dotCollected) continue;
        float cx = p.x + p.w * 0.5f;
        float dotR = std::max(6.0f, std::min(p.w, p.h) * 0.11f);
        float cy = p.y + p.h + dotR * 1.2f;
        float dx = screenGhostX - cx;
        float dy = ghost.y - cy;
        float collectR = std::max(22.0f, std::min(p.w, p.h) * 0.24f) + ghost.radius * 0.35f;
        if (dx * dx + dy * dy < collectR * collectR) {
            p.dotCollected = true;
            level.dotsCollectedTotal++;
            level.blackDotSpawnCounter++;
            OnDotCollected();
            // Spawn black dot platform based on counter reaching target
            // Prevent spawn if win condition (20 dots) has been met
            if (level.dotsCollectedTotal < 20 && level.blackDotSpawnCounter >= level.blackDotNextSpawnAt) {
                level.SpawnBlackBorderPlatform((int)screenW, ghost.x, ghost.y);
                level.blackDotsSpawnedCount++;
                level.hasFirstBlackDotSpawned = true;
                level.blackDotSpawnCounter = 0;
                // After first spawn (at 10), use range 2-4 (meaning values between 2 and 4)
                level.blackDotNextSpawnAt = 2 + (rand() % 3); // 2, 3, or 4
            }
        }
    }

    for (auto& s : level.sideLedges) {
        if (s.blackDotCollected) continue;
        float cx = s.x + s.w * 0.5f;
        float dotR = std::max(11.0f, std::min(s.w, s.h) * 0.18f);
        float cy = s.y + s.h + dotR * 1.25f;
        float dx = screenGhostX - cx;
        float dy = ghost.y - cy;
        float collectR = std::max(28.0f, std::min(s.w, s.h) * 0.30f) + ghost.radius * 0.4f;
        if (dx * dx + dy * dy < collectR * collectR) {
            s.blackDotCollected = true;
            ApplyBlackDotWallRelief(screenW);
        }
    }

    const float passMargin = ghost.radius + 14.0f;
    for (auto& p : level.platforms) {
        if (!p.hasDot || p.dotCollected || p.skipRegistered) continue;
        if (ghost.y > p.y + p.h + passMargin) {
            p.skipRegistered = true;
            OnDotSkipped();
        }
    }
}

bool WallSystem::IsGhostCrushed(const GhostController& ghost) const {
    const float tipL = leftX + kBladeDepth;
    const float tipR = rightX - kBladeDepth;
    return ghost.x - ghost.radius <= tipL + 1.5f || ghost.x + ghost.radius >= tipR - 1.5f;
}

void WallSystem::Draw(float screenW, float screenH) const {
    glDisable(GL_BLEND);
    // Near-black void walls with subtle warmth
    glColor3f(0.04f, 0.025f, 0.03f);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(leftX, 0.0f);
    glVertex2f(leftX, screenH);
    glVertex2f(0.0f, screenH);

    glVertex2f(rightX, 0.0f);
    glVertex2f(screenW, 0.0f);
    glVertex2f(screenW, screenH);
    glVertex2f(rightX, screenH);
    glEnd();

    glColor3f(0.12f, 0.08f, 0.07f);
    glBegin(GL_QUADS);
    glVertex2f(leftX - 3.0f, 0.0f);
    glVertex2f(leftX + 2.0f, 0.0f);
    glVertex2f(leftX + 2.0f, screenH);
    glVertex2f(leftX - 3.0f, screenH);

    glVertex2f(rightX - 2.0f, 0.0f);
    glVertex2f(rightX + 3.0f, 0.0f);
    glVertex2f(rightX + 3.0f, screenH);
    glVertex2f(rightX - 2.0f, screenH);
    glEnd();

    // Inward-pointing blade triangles (toward playfield): left wall tips +X, right wall tips -X
    const float bladeH = 22.0f;
    const float bladeDepth = kBladeDepth;
    const float step = 36.0f;
    glColor3f(0.18f, 0.14f, 0.13f);
    glBegin(GL_TRIANGLES);
    for (float yy = 0.0f; yy + bladeH <= screenH + 1.0f; yy += step) {
        float y0 = yy;
        float y1 = std::min(yy + bladeH, screenH);
        float ym = (y0 + y1) * 0.5f;
        // Left inner edge at leftX: apex points right into arena
        glVertex2f(leftX, y0);
        glVertex2f(leftX, y1);
        glVertex2f(leftX + bladeDepth, ym);

        // Right inner edge at rightX: apex points left
        glVertex2f(rightX, y0);
        glVertex2f(rightX, y1);
        glVertex2f(rightX - bladeDepth, ym);
    }
    glEnd();
    glColor3f(0.28f, 0.22f, 0.20f);
    glBegin(GL_LINES);
    for (float yy = 0.0f; yy + bladeH <= screenH + 1.0f; yy += step) {
        float y0 = yy;
        float y1 = std::min(yy + bladeH, screenH);
        float ym = (y0 + y1) * 0.5f;
        glVertex2f(leftX, y0);
        glVertex2f(leftX + bladeDepth, ym);
        glVertex2f(leftX + bladeDepth, ym);
        glVertex2f(leftX, y1);
        glVertex2f(rightX, y0);
        glVertex2f(rightX - bladeDepth, ym);
        glVertex2f(rightX - bladeDepth, ym);
        glVertex2f(rightX, y1);
    }
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
}

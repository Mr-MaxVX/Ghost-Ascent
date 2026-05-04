#include <GL/glew.h>
#include "LevelGenerator.hpp"
#include "BrickRenderer.hpp"
#include <cmath>
#include <algorithm>

namespace {

// Mirror GhostController jump/move numbers — keep in sync when tuning movement.
constexpr float kG = 520.0f;
constexpr float kJ1 = 360.0f;
constexpr float kJ2 = 360.0f * 0.88f;
constexpr float kVmax = 260.0f;

// Vertical rise (platform top to next platform top) with ideal single / double jump.
inline float MaxSingleJumpTopDelta() {
    return std::max(40.0f, (kJ1 * kJ1) / (2.0f * kG) - 14.0f);
}

inline float MaxDoubleJumpTopDelta() {
    return std::max(80.0f, (kJ1 * kJ1 + kJ2 * kJ2) / (2.0f * kG) - 20.0f);
}

// Rough horizontal reach while climbing ~dy: airtime scales with how tall the hop is.
inline float MaxLateralForTopDelta(float dy) {
    float riseTime = kJ1 / kG + kJ2 / kG;
    float extraFall = std::sqrt(std::max(0.0f, dy - 40.0f) / kG) * 0.85f;
    float t = (riseTime + extraFall + 0.28f) * 0.92f;
    return std::min(kVmax * t, 290.0f);
}

inline float Rnd01(unsigned& seed) {
    seed = seed * 1103515245u + 12345u;
    return ((seed >> 16) & 0x7fffu) / 32767.0f;
}

} // namespace

float LevelGenerator::PlatformWidthForWorldY(float worldY) const {
    float h = std::max(0.0f, worldY);
    float w = kStartScale - h * kScalingFactor;
    return std::max(kMinScale, w);
}

void LevelGenerator::BuildInitial(int screenWidth, float groundTop) {
    platforms.clear();
    sideLedges.clear();
    maxHeightReached = 0.0f;
    dotsCollectedTotal = 0;
    nextBlackPlatformOnLeft = true;
    blackDotSpawnCounter = 0;
    blackDotNextSpawnAt = 10; // First spawn at 10 dots
    hasFirstBlackDotSpawned = false;
    blackDotsSpawnedCount = 0;

    // Spawn initial side platforms with black dots at various heights
    const float edgeMargin = 48.0f;
    float heights[] = { 250.0f, 550.0f, 950.0f, 1400.0f, 1900.0f };
    for (size_t i = 0; i < 5; ++i) {
        SideBlackLedge L;
        L.h = 26.0f;
        L.w = 90.0f;
        L.isLeft = (i % 2 == 0);
        L.y = groundTop + heights[i];
        if (L.isLeft)
            L.x = edgeMargin;
        else
            L.x = (float)screenWidth - L.w - edgeMargin;
        L.blackDotCollected = false;
        sideLedges.push_back(L);
    }
    nextBlackPlatformOnLeft = false;

    const float margin = 72.0f;
    const int count = 48;
    unsigned seed = 54321u;

    const float maxSingle = MaxSingleJumpTopDelta();
    const float maxDouble = MaxDoubleJumpTopDelta();
    const float minStep = 96.0f; // edgy lower bound — usually eats most of first jump

    // Previous surface top (world y); start at ground slab top.
    float prevTop = groundTop;
    float prevCx = screenWidth * 0.5f;
    int zig = (Rnd01(seed) > 0.5f) ? 1 : -1;

    for (int i = 0; i < count; ++i) {
        DashPlatform p;
        p.h = 24.0f + Rnd01(seed) * 10.0f;

        float dy = 0.0f;
        if (i == 0) {
            // Surface-to-surface delta from ground must stay within a single jump from standing.
            float u = Rnd01(seed);
            float t = 0.28f + 0.72f * std::pow(u, 0.82f);
            dy = 58.0f + (maxSingle - 26.0f) * t;
            dy = std::min(dy, maxSingle - 12.0f);
        } else {
            // Bias toward tall steps (needs double jump) without exceeding physics cap.
            float u = Rnd01(seed);
            float edgy = std::pow(u, 0.72f);
            float cap = maxDouble;
            float low = minStep;
            if (Rnd01(seed) < 0.22f) {
                // Occasional “single-jump window” — still tight horizontally
                cap = std::min(cap, maxSingle + 18.0f);
                low = std::max(72.0f, maxSingle * 0.55f);
            }
            dy = low + (cap - low) * edgy;
            dy = std::max(low, std::min(cap, dy));
        }

        float targetTop = prevTop + dy;
        float maxLat = MaxLateralForTopDelta(dy) * (0.78f + 0.22f * Rnd01(seed));

        zig = -zig;
        float side = zig * (0.38f + 0.62f * Rnd01(seed));
        float wantCx = prevCx + side * maxLat;

        p.w = PlatformWidthForWorldY(targetTop - p.h * 0.5f);
        float half = p.w * 0.5f;
        float cxMin = margin + half + 6.0f;
        float cxMax = screenWidth - margin - half - 6.0f;
        float cx = std::max(cxMin, std::min(cxMax, wantCx));

        // If clamp killed lateral reach, lower the step a bit so the arc can still make it
        float lost = std::fabs(wantCx - cx);
        if (lost > 35.0f) targetTop -= std::min(20.0f, lost * 0.11f);

        p.y = targetTop - p.h;
        p.x = cx - half;

        p.hasDot = true;
        p.dotCollected = false;
        p.skipRegistered = false;
        platforms.push_back(p);

        prevTop = p.y + p.h;
        prevCx = cx;
    }
}

void LevelGenerator::SpawnBlackBorderPlatform(int screenW, float ghostX, float ghostY) {
    if (platforms.empty() || screenW <= 0) return;

    size_t best = 0;
    float bestD = 1.0e30f;
    for (size_t i = 0; i < platforms.size(); ++i) {
        const auto& p = platforms[i];
        float cx = p.x + p.w * 0.5f;
        float cy = p.y + p.h * 0.5f;
        float dx = ghostX - cx;
        float dy = ghostY - cy;
        float d = dx * dx + dy * dy;
        if (d < bestD) {
            bestD = d;
            best = i;
        }
    }

    const auto& ref = platforms[best];
    SideBlackLedge L;
    L.h = ref.h;
    L.w = std::max(70.0f, std::min(ref.w * 0.55f, 130.0f));
    L.isLeft = nextBlackPlatformOnLeft;
    nextBlackPlatformOnLeft = !nextBlackPlatformOnLeft;
    
    // Spawn near center with offset, not at edges
    const float center = screenW * 0.5f;
    const float offsetMin = 100.0f;
    const float offsetMax = 250.0f;
    float offset = offsetMin + (float)(rand() % 1000) / 1000.0f * (offsetMax - offsetMin);
    
    if (L.isLeft)
        L.x = center - offset - L.w;
    else
        L.x = center + offset;
    
    L.y = ref.y;
    L.blackDotCollected = false;
    sideLedges.push_back(L);
}

void LevelGenerator::UpdateMaxHeight(float ghostY) {
    if (ghostY > maxHeightReached) maxHeightReached = ghostY;
}

std::vector<AABB> LevelGenerator::PlatformColliders() const {
    std::vector<AABB> out;
    out.reserve(platforms.size() + sideLedges.size());
    for (const auto& p : platforms) {
        AABB b{ p.x, p.y, p.w, p.h };
        out.push_back(b);
    }
    for (const auto& s : sideLedges) {
        AABB b{ s.x, s.y, s.w, s.h };
        out.push_back(b);
    }
    return out;
}

void LevelGenerator::DrawGround(float worldW, float groundTop) const {
    DrawDetailedBrickRect(0.0f, 0.0f, worldW, groundTop, brickSalt);
}

void LevelGenerator::DrawPlatforms(float cameraY, float viewH, float timeSec) const {
    const float pad = 120.0f;
    float y0 = cameraY - pad;
    float y1 = cameraY + viewH + pad;

    for (const auto& p : platforms) {
        if (p.y + p.h < y0 || p.y > y1) continue;
        DrawDetailedBrickRect(p.x, p.y, p.w, p.h, brickSalt + (unsigned)(p.y * 0.01f));
        if (p.hasDot && !p.dotCollected) {
            float cx = p.x + p.w * 0.5f;
            float dotR = std::max(6.0f, std::min(p.w, p.h) * 0.11f);
            // Sit on top of the brick, centered — not embedded in the middle
            float cy = p.y + p.h + dotR * 1.2f;
            DrawShineDot(cx, cy, dotR, timeSec);
        }
    }

    for (const auto& s : sideLedges) {
        if (s.y + s.h < y0 || s.y > y1) continue;
        DrawDetailedBrickRect(s.x, s.y, s.w, s.h, brickSalt + 777u + (unsigned)(s.y * 0.02f));
        if (!s.blackDotCollected) {
            float cx = s.x + s.w * 0.5f;
            float dotR = std::max(18.0f, std::min(s.w, s.h) * 0.28f);
            float cy = s.y + s.h + dotR * 1.4f;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // White outer glow (aura)
            glColor4f(1.0f, 1.0f, 1.0f, 0.4f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            const int n = 24;
            for (int i = 0; i <= n; ++i) {
                float a = (6.2831853f * i) / n;
                glVertex2f(cx + std::cos(a) * dotR * 1.4f, cy + std::sin(a) * dotR * 1.4f);
            }
            glEnd();

            // White solid ring (outer border)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_TRIANGLE_STRIP);
            for (int i = 0; i <= n; ++i) {
                float a = (6.2831853f * i) / n;
                float c = std::cos(a), s2 = std::sin(a);
                glVertex2f(cx + c * dotR * 0.85f, cy + s2 * dotR * 0.85f);
                glVertex2f(cx + c * dotR, cy + s2 * dotR);
            }
            glEnd();

            // Black middle circle
            glColor4f(0.02f, 0.02f, 0.04f, 1.0f);
            float midR = dotR * 0.75f;
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (int i = 0; i <= n; ++i) {
                float a = (6.2831853f * i) / n;
                glVertex2f(cx + std::cos(a) * midR, cy + std::sin(a) * midR);
            }
            glEnd();

            // White inner core (bright center)
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            float innerR = dotR * 0.45f;
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(cx, cy);
            for (int i = 0; i <= n; ++i) {
                float a = (6.2831853f * i) / n;
                glVertex2f(cx + std::cos(a) * innerR, cy + std::sin(a) * innerR);
            }
            glEnd();

            glDisable(GL_BLEND);
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}



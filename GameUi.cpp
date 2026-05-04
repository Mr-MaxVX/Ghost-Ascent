#include "GameUi.hpp"
#include <GL/glew.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <cstdint>

namespace {

void DrawSevenSegDigit(float x, float y, float digitH, int digit) {
    static const unsigned char pat[10] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F };
    float hh = digitH;
    float ww = hh * 0.58f;
    float t = std::max(2.5f, hh * 0.13f);
    float x0 = x + t;
    float x1 = x + ww - t;
    float y0 = y + t;
    float y1 = y + hh - t;
    float ym = y + hh * 0.5f;
    unsigned char m = pat[digit % 10];

    glBegin(GL_QUADS);
    if (m & 0x01) {
        glVertex2f(x0, y1 - t);
        glVertex2f(x1, y1 - t);
        glVertex2f(x1, y1);
        glVertex2f(x0, y1);
    }
    if (m & 0x02) {
        float ya = ym + t * 0.35f;
        float yb = y1 - t;
        glVertex2f(x1, ya);
        glVertex2f(x1 + t, ya);
        glVertex2f(x1 + t, yb);
        glVertex2f(x1, yb);
    }
    if (m & 0x04) {
        float ya = y0 + t;
        float yb = ym - t * 0.35f;
        glVertex2f(x1, ya);
        glVertex2f(x1 + t, ya);
        glVertex2f(x1 + t, yb);
        glVertex2f(x1, yb);
    }
    if (m & 0x08) {
        glVertex2f(x0, y0);
        glVertex2f(x1, y0);
        glVertex2f(x1, y0 + t);
        glVertex2f(x0, y0 + t);
    }
    if (m & 0x10) {
        float ya = y0 + t;
        float yb = ym - t * 0.35f;
        glVertex2f(x0 - t, ya);
        glVertex2f(x0, ya);
        glVertex2f(x0, yb);
        glVertex2f(x0 - t, yb);
    }
    if (m & 0x20) {
        float ya = ym + t * 0.35f;
        float yb = y1 - t;
        glVertex2f(x0 - t, ya);
        glVertex2f(x0, ya);
        glVertex2f(x0, yb);
        glVertex2f(x0 - t, yb);
    }
    if (m & 0x40) {
        glVertex2f(x0, ym - t * 0.5f);
        glVertex2f(x1, ym - t * 0.5f);
        glVertex2f(x1, ym + t * 0.5f);
        glVertex2f(x0, ym + t * 0.5f);
    }
    glEnd();
}
} // namespace

void DrawPlayerRadialLight(float px, float py, float ghostRadius, int dotsGathered, float timeSec, bool hasFirstBlackDotSpawned) {
    // Light ONLY appears after first black-white dot has spawned (after 10 white dots)
    // Adjusted for 20-dot win condition
    float alpha = 0.0f;
    if (!hasFirstBlackDotSpawned) {
        alpha = 0.0f; // No light until first black dot spawns
    } else if (dotsGathered < 15) {
        // Fade in from 10 to 14
        float t = (float)(dotsGathered - 10) / 5.0f;
        alpha = t * 0.4f; // Max 0.4 at 14 dots
    } else if (dotsGathered < 20) {
        // Intensely bright from 15 to 19
        float t = (float)(dotsGathered - 15) / 4.0f;
        alpha = 0.4f + t * 0.6f; // 0.4 to 1.0
    } else {
        alpha = 1.0f; // At 20+ dots, full brightness
    }
    
    const float hope = std::min(1.0f, dotsGathered / 48.0f);
    const float pulse = 0.94f + 0.06f * std::sin(timeSec * 2.1f);
    const float R = ghostRadius * (3.8f + 3.5f * hope + alpha * 2.0f); // Expand radius with alpha
    
    // Early out if completely invisible
    if (alpha <= 0.001f) return;

    glEnable(GL_BLEND);

    const int rings = 7;
    for (int ri = rings; ri >= 1; --ri) {
        float rr = R * (float)ri / (float)rings;
        float ringT = 1.0f - (float)(ri - 1) / (float)rings;
        float a = (0.045f + 0.11f * hope) * ringT * pulse * alpha; // Apply phase alpha
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(0.82f, 0.90f, 1.0f, a);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px, py);
        const int n = 36;
        for (int i = 0; i <= n; ++i) {
            float ang = (6.2831853f * i) / n;
            glVertex2f(px + std::cos(ang) * rr, py + std::sin(ang) * rr);
        }
        glEnd();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glColor4f(0.95f, 0.97f, 1.0f, (0.07f + 0.12f * hope) * pulse * alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(px, py);
    const int n2 = 28;
    for (int i = 0; i <= n2; ++i) {
        float ang = (6.2831853f * i) / n2;
        glVertex2f(px + std::cos(ang) * ghostRadius * 1.35f, py + std::sin(ang) * ghostRadius * 1.35f);
    }
    glEnd();

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawTopFixtureArc(float cx, float topY, float viewW, float timeSec, int dotsGathered) {
    // Only appear after 10 dots
    if (dotsGathered < 10) return;
    
    // Smooth fade-in from 10 to 15 dots
    float intensity = std::min(1.0f, (dotsGathered - 9) / 5.0f);
    
    const float R = viewW * 0.46f;
    const float pulse = 0.91f + 0.09f * std::sin(timeSec * 1.65f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 0.99f, 0.96f, 0.34f * pulse * intensity);
    glVertex2f(cx, topY);
    const int n = 150; // Much smoother arc
    for (int i = 0; i <= n; ++i) {
        float t = 3.14159265f * (float)i / (float)n;
        float x = cx + std::cos(t) * R;
        float y = topY - std::sin(t) * R;
        glColor4f(0.95f, 0.94f, 0.92f, 0.05f * pulse * intensity);
        glVertex2f(x, y);
    }
    glEnd();
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawWhiteDotCountHud(float scrW, float scrH, int whiteDots) {
    whiteDots = std::max(0, std::min(whiteDots, 999));
    float digH = std::max(22.0f, scrH * 0.036f);
    float step = digH * 0.72f;
    int d0 = whiteDots / 100;
    int d1 = (whiteDots / 10) % 10;
    int d2 = whiteDots % 10;
    int nd = whiteDots >= 100 ? 3 : (whiteDots >= 10 ? 2 : 1);
    float totalW = nd * step;
    float ox = scrW - 28.0f - totalW;
    float oy = scrH - digH - 26.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.96f, 0.96f, 1.0f, 0.94f);
    float x = ox;
    if (nd == 3) {
        DrawSevenSegDigit(x, oy, digH, d0);
        x += step;
    }
    if (nd >= 2) {
        DrawSevenSegDigit(x, oy, digH, d1);
        x += step;
    }
    DrawSevenSegDigit(x, oy, digH, d2);
    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void DrawBloodOverlay(float w, float h, float strength, float timeSec) {
    if (strength <= 0.02f) return;

    const float pulse = 0.90f + 0.10f * std::sin(timeSec * 6.0f);
    const float s = strength * pulse;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark red base vignette
    glColor4f(0.78f, 0.02f, 0.05f, 0.45f * s);
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(w, 0.0f);
    glVertex2f(w, h);
    glVertex2f(0.0f, h);
    glEnd();

    // Blood drips from top - procedural jagged strips
    glColor4f(0.55f, 0.0f, 0.02f, 0.85f * s);
    const int numDrips = 32;
    const float stripW = w / numDrips;

    for (int i = 0; i < numDrips; ++i) {
        // Pseudo-random drip length based on position and time
        float hash = std::sin(i * 12.9898f + timeSec * 0.5f) * 43758.5453f;
        hash = hash - std::floor(hash);
        float dripLen = h * (0.15f + hash * 0.35f) * s;

        float x0 = i * stripW;
        float x1 = x0 + stripW * 0.7f;
        float y1 = dripLen;

        // Jagged drip shape
        float xm = (x0 + x1) * 0.5f;
        float ym = dripLen * (0.7f + hash * 0.3f);

        glBegin(GL_TRIANGLES);
        glVertex2f(x0, 0.0f);
        glVertex2f(x1, 0.0f);
        glVertex2f(xm, ym);
        glEnd();

        // Thicker top bar
        glColor4f(0.35f, 0.0f, 0.01f, 0.95f * s);
        glBegin(GL_QUADS);
        glVertex2f(x0 - stripW * 0.15f, 0.0f);
        glVertex2f(x1 + stripW * 0.15f, 0.0f);
        glVertex2f(x1 + stripW * 0.15f, h * 0.06f);
        glVertex2f(x0 - stripW * 0.15f, h * 0.06f);
        glEnd();
        glColor4f(0.55f, 0.0f, 0.02f, 0.85f * s);
    }

    // Blood splatter droplets falling
    glColor4f(0.65f, 0.0f, 0.03f, 0.6f * s);
    for (int i = 0; i < 24; ++i) {
        float hashX = std::sin(i * 17.351f) * 43758.5453f;
        hashX = hashX - std::floor(hashX);
        float hashY = std::sin(i * 23.714f + timeSec) * 43758.5453f;
        hashY = hashY - std::floor(hashY);

        float dropX = hashX * w;
        float dropY = (hashY * h * 0.6f) + h * 0.1f;
        float dropR = 3.0f + hashX * 6.0f;

        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(dropX, dropY);
        for (int j = 0; j <= 8; ++j) {
            float a = (6.2831853f * j) / 8;
            glVertex2f(dropX + std::cos(a) * dropR, dropY + std::sin(a) * dropR);
        }
        glEnd();
    }

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

// ============================================================================
// Black Feather Shards Particle System (for Game Won state)
// ============================================================================
namespace {
struct BlackShard {
    float x, y;
    float speedY;
    float phaseOffset;
    float size;
};
std::vector<BlackShard> g_shards;
float g_lastShardSpawn = 0.0f;
const float kSwayAmplitude = 30.0f;
}

void UpdateAndDrawBlackFeatherShards(float screenW, float screenH, float dt, float timeSec) {
    // Spawn new shards continuously
    g_lastShardSpawn += dt;
    if (g_lastShardSpawn > 0.08f) { // Spawn ~12 shards per second
        g_lastShardSpawn = 0.0f;
        BlackShard s;
        s.x = (float)(rand() % (int)screenW);
        s.y = -20.0f; // Just above screen
        s.speedY = 80.0f + (float)(rand() % 150); // 80-230 px/s
        s.phaseOffset = (float)(rand() % 1000) / 100.0f;
        s.size = 4.0f + (float)(rand() % 8); // 4-12px
        g_shards.push_back(s);
    }

    // Update and draw
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor3f(0.0f, 0.0f, 0.0f);

    for (auto it = g_shards.begin(); it != g_shards.end(); ) {
        // Update position
        it->y += it->speedY * dt;
        float sway = std::sin(timeSec + it->phaseOffset) * kSwayAmplitude * dt;
        it->x += sway;

        // Draw jagged shard (triangle with irregular shape)
        float sz = it->size;
        float tipY = it->y - sz * 1.5f;
        float baseY = it->y + sz * 0.5f;
        float leftX = it->x - sz;
        float rightX = it->x + sz;

        glBegin(GL_TRIANGLES);
        // Main jagged shard shape
        glVertex2f(it->x, tipY); // Tip pointing up
        glVertex2f(leftX, baseY);  // Left base
        glVertex2f(rightX, baseY); // Right base
        glEnd();

        // Small extra triangle for jagged look
        float midX = it->x + sz * 0.3f;
        float midY = (tipY + baseY) * 0.5f;
        glBegin(GL_TRIANGLES);
        glVertex2f(midX, midY);
        glVertex2f(rightX + sz * 0.3f, midY + sz * 0.5f);
        glVertex2f(rightX, baseY - sz * 0.3f);
        glEnd();

        // Remove if below screen
        if (it->y > screenH + 50.0f) {
            it = g_shards.erase(it);
        } else {
            ++it;
        }
    }

    glColor3f(1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);
}

void ClearBlackFeatherShards() {
    g_shards.clear();
    g_lastShardSpawn = 0.0f;
}

// ============================================================================
// Clean Outline Font (wireframe block letters)
// ============================================================================
namespace {
    void DrawOutlineChar(float x, float y, char c, float w, float h, float thick, float r, float g, float b) {
        float lx = x;
        float rx = x + w;
        float by = y;
        float ty = y - h;
        float midY = y - h * 0.5f;
        float midX = x + w * 0.5f;
        
        glColor3f(r, g, b);
        glLineWidth(thick);
        glBegin(GL_LINES);
        
        switch(c) {
            case 'A':
                glVertex2f(lx, by); glVertex2f(midX, ty);
                glVertex2f(rx, by); glVertex2f(midX, ty);
                glVertex2f(lx + w*0.25f, midY); glVertex2f(rx - w*0.25f, midY);
                break;
            case 'C':
                glVertex2f(rx, ty); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                break;
            case 'D':
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, ty); glVertex2f(midX, ty + h*0.1f);
                glVertex2f(lx, by); glVertex2f(midX, by - h*0.1f);
                glVertex2f(midX, ty + h*0.1f); glVertex2f(rx, midY);
                glVertex2f(midX, by - h*0.1f); glVertex2f(rx, midY);
                break;
            case 'E':
                glVertex2f(rx, ty); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                glVertex2f(lx, midY); glVertex2f(rx - w*0.2f, midY);
                break;
            case 'G':
                glVertex2f(rx, ty); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                glVertex2f(rx, by); glVertex2f(rx, midY);
                glVertex2f(rx, midY); glVertex2f(midX, midY);
                break;
            case 'H':
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(rx, ty); glVertex2f(rx, by);
                glVertex2f(lx, midY); glVertex2f(rx, midY);
                break;
            case 'I':
                glVertex2f(midX, ty); glVertex2f(midX, by);
                break;
            case 'L':
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                break;
            case 'M':
                glVertex2f(lx, by); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(midX, midY);
                glVertex2f(midX, midY); glVertex2f(rx, ty);
                glVertex2f(rx, ty); glVertex2f(rx, by);
                break;
            case 'N':
                glVertex2f(lx, by); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(rx, by);
                glVertex2f(rx, by); glVertex2f(rx, ty);
                break;
            case 'O':
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                glVertex2f(rx, by); glVertex2f(rx, ty);
                glVertex2f(rx, ty); glVertex2f(lx, ty);
                break;
            case 'P':
                glVertex2f(lx, by); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(rx, ty + h*0.3f);
                glVertex2f(rx, ty + h*0.3f); glVertex2f(rx, midY);
                glVertex2f(rx, midY); glVertex2f(lx, midY);
                break;
            case 'R':
                glVertex2f(lx, by); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(rx, ty + h*0.3f);
                glVertex2f(rx, ty + h*0.3f); glVertex2f(rx, midY);
                glVertex2f(rx, midY); glVertex2f(lx, midY);
                glVertex2f(lx, midY); glVertex2f(rx, by);
                break;
            case 'S':
                glVertex2f(rx, ty); glVertex2f(lx, ty);
                glVertex2f(lx, ty); glVertex2f(lx, midY);
                glVertex2f(lx, midY); glVertex2f(rx, midY);
                glVertex2f(rx, midY); glVertex2f(rx, by);
                glVertex2f(rx, by); glVertex2f(lx, by);
                break;
            case 'T':
                glVertex2f(lx, ty); glVertex2f(rx, ty);
                glVertex2f(midX, ty); glVertex2f(midX, by);
                break;
            case 'U':
                glVertex2f(lx, ty); glVertex2f(lx, by);
                glVertex2f(lx, by); glVertex2f(rx, by);
                glVertex2f(rx, by); glVertex2f(rx, ty);
                break;
            case 'V':
                glVertex2f(lx, ty); glVertex2f(midX, by);
                glVertex2f(rx, ty); glVertex2f(midX, by);
                break;
            case 'W':
                glVertex2f(lx, ty); glVertex2f(lx + w*0.25f, by);
                glVertex2f(lx + w*0.25f, by); glVertex2f(midX, midY);
                glVertex2f(midX, midY); glVertex2f(rx - w*0.25f, by);
                glVertex2f(rx - w*0.25f, by); glVertex2f(rx, ty);
                break;
            case 'Y':
                glVertex2f(lx, ty); glVertex2f(midX, midY);
                glVertex2f(rx, ty); glVertex2f(midX, midY);
                glVertex2f(midX, midY); glVertex2f(midX, by);
                break;
            case ' ':
                break;
        }
        glEnd();
        glLineWidth(1.0f);
    }

    void DrawOutlineString(float x, float y, const char* text, float letterW, float letterH, float thick, float r, float g, float b, int maxChars) {
        float spacing = letterW * 1.3f;
        float cx = x;
        for (int i = 0; text[i] && i < maxChars; ++i) {
            if (text[i] == ' ') {
                cx += letterW * 0.8f;
            } else {
                DrawOutlineChar(cx, y, text[i], letterW, letterH, thick, r, g, b);
                cx += spacing;
            }
        }
    }

    float OutlineStringWidth(const char* text, float letterW) {
        float spacing = letterW * 1.3f;
        float width = 0;
        int chars = 0;
        for (int i = 0; text[i]; ++i) {
            if (text[i] != ' ') chars++;
            else width += letterW * 0.8f;
        }
        return chars * spacing + (chars > 0 ? -spacing * 0.3f : 0);
    }
}

// ============================================================================
// Drawing Game Ended text (black on white background) "YOU WON"
// ============================================================================
void DrawGameEndedText(float screenW, float screenH) {
    const char* text = "YOU WON";
    float letterW = screenW * 0.06f;
    float letterH = screenH * 0.1f;
    float thick = 3.0f;
    float textW = OutlineStringWidth(text, letterW);
    float x = (screenW - textW) * 0.5f;
    float y = screenH * 0.55f;
    DrawOutlineString(x, y, text, letterW, letterH, thick, 0.0f, 0.0f, 0.0f, 999);
}

// ============================================================================
// Drawing Game Over text (red on blood background) "GAME OVER"
// ============================================================================
void DrawGameOverText(float screenW, float screenH) {
    const char* text = "GAME OVER";
    float letterW = screenW * 0.05f;
    float letterH = screenH * 0.1f;
    float thick = 3.0f;
    float textW = OutlineStringWidth(text, letterW);
    float x = (screenW - textW) * 0.5f;
    float y = screenH * 0.55f;
    DrawOutlineString(x, y, text, letterW, letterH, thick, 0.9f, 0.0f, 0.0f, 999);
}

// ============================================================================
// Drawing Intro Text Box (for cinematic intro sequence)
// ============================================================================
void DrawIntroTextBox(float screenW, float screenH, int charsRevealed) {
    const float boxW = screenW * 0.6f;
    const float boxH = screenH * 0.15f;
    const float boxX = screenW * 0.5f - boxW * 0.5f;
    const float boxY = screenH * 0.75f;
    const float borderThick = 3.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Dark semi-transparent background
    glColor4f(0.05f, 0.02f, 0.02f, 0.85f);
    glBegin(GL_QUADS);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH);
    glVertex2f(boxX, boxY + boxH);
    glEnd();

    // Dark red border
    glColor3f(0.4f, 0.0f, 0.0f);
    glLineWidth(borderThick);
    glBegin(GL_LINE_LOOP);
    glVertex2f(boxX, boxY);
    glVertex2f(boxX + boxW, boxY);
    glVertex2f(boxX + boxW, boxY + boxH);
    glVertex2f(boxX, boxY + boxH);
    glEnd();
    glLineWidth(1.0f);

    // Draw text with typewriter effect
    float letterW = boxW * 0.035f;
    float letterH = boxH * 0.35f;
    float thick = 2.0f;
    
    const char* line1 = "DISCIPLINE IS SAVIOR";
    const char* line2 = "GREED IS DEATH";
    int len1 = 20; // Length of line 1
    int drawCount1 = std::min(charsRevealed, len1);
    int drawCount2 = std::max(0, charsRevealed - len1);
    
    float w1 = OutlineStringWidth(line1, letterW);
    float x1 = boxX + (boxW - w1) * 0.5f;
    float y1 = boxY + boxH * 0.65f;
    DrawOutlineString(x1, y1, line1, letterW, letterH, thick, 0.9f, 0.9f, 0.9f, drawCount1);
    
    float w2 = OutlineStringWidth(line2, letterW);
    float x2 = boxX + (boxW - w2) * 0.5f;
    float y2 = boxY + boxH * 0.35f;
    DrawOutlineString(x2, y2, line2, letterW, letterH, thick, 0.9f, 0.9f, 0.9f, drawCount2);

    glDisable(GL_BLEND);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

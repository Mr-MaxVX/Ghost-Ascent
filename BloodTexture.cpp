#include "BloodTexture.hpp"
#include <GL/glew.h>
#include <cstdio>
#include <cstring>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {

GLuint gTex = 0;
int gW = 0;
int gH = 0;

void KeyGreenAndBlackToAlpha(unsigned char* rgba, int w, int h) {
    for (int i = 0; i < w * h; ++i) {
        unsigned char* p = rgba + i * 4;
        float r = p[0] / 255.0f, g = p[1] / 255.0f, b = p[2] / 255.0f;
        // Green-screen
        if (g > 0.35f && g > r * 1.15f && g > b * 1.15f) {
            p[3] = 0;
            continue;
        }
        // Near-black background → transparent
        if (r < 0.06f && g < 0.06f && b < 0.06f) {
            p[3] = 0;
            continue;
        }
    }
}

bool TryLoadPath(const char* path) {
    int w = 0, h = 0, n = 0;
    unsigned char* data = stbi_load(path, &w, &h, &n, 4);
    if (!data || w <= 0 || h <= 0) {
        if (data) stbi_image_free(data);
        return false;
    }

    KeyGreenAndBlackToAlpha(data, w, h);

    if (gTex == 0) glGenTextures(1, &gTex);
    glBindTexture(GL_TEXTURE_2D, gTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    gW = w;
    gH = h;
    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

} // namespace

bool LoadBloodTexture() {
    if (gTex != 0) return true;

    static const char* paths[] = {
        "Circle/assets/blood_drip.png",
        "assets/blood_drip.png",
        "..\\Circle\\assets\\blood_drip.png",
        "..\\..\\Circle\\assets\\blood_drip.png",
    };
    for (const char* p : paths) {
        if (TryLoadPath(p)) return true;
    }
    return false;
}

void UnloadBloodTexture() {
    if (gTex != 0) {
        glDeleteTextures(1, &gTex);
        gTex = 0;
    }
    gW = gH = 0;
}

bool IsBloodTextureLoaded() {
    return gTex != 0;
}

void DrawBloodTextureLayer(float screenW, float screenH, float alpha) {
    if (gTex == 0 || alpha <= 0.02f) return;

    float drawH = screenH * 0.52f;
    float drawW = screenW;
    // Render at TOP of screen (blood dripping down)
    float y0 = 0.0f;
    float y1 = drawH;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, gTex);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    glBegin(GL_QUADS);
    // Flip UVs so blood drips from top
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(0.0f, y0);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(drawW, y0);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(drawW, y1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(0.0f, y1);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

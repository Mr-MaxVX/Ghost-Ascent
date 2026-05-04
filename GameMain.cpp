#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "MonitorManager.hpp"
#include "GhostController.hpp"
#include "LevelGenerator.hpp"
#include "WallSystem.hpp"
#include "GameUi.hpp"
#include "BloodTexture.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace {
constexpr float kGroundTop = 100.0f;

enum class GameState {
    Intro_Idle,
    Intro_ZoomIn,
    Intro_Text,
    Intro_ZoomOut,
    Playing,
    Win_Sequence
};

float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

int RunGhostAscent() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = CreatePrimaryBorderlessWindow("Ghost Ascent");
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        return -1;
    }
    glGetError();

    LoadBloodTexture();

    GhostController ghost;
    LevelGenerator level;
    WallSystem walls;

    bool gameOver = false;
    bool isGameWon = false;
    bool rHeld = false;
    float bloodAmount = 0.0f;
    
    // Intro sequence state
    GameState currentState = GameState::Intro_Idle;
    float currentZoom = 1.0f;
    float zoomTimer = 0.0f;
    bool spaceHeld = false;
    float textRevealTimer = 0.0f;
    
    // Win sequence variables
    float winTimer = 0.0f;
    float winStartY = 0.0f;
    float winCamY = 0.0f;

    auto restart = [&](int fbW, int fbH) {
        level.BuildInitial(fbW, kGroundTop);
        walls.Reset((float)fbW);
        ghost.Reset((float)fbW * 0.5f, kGroundTop + ghost.radius + 42.0f);
        level.maxHeightReached = 0.0f;
        gameOver = false;
        isGameWon = false;
        bloodAmount = 0.0f;
        ClearBlackFeatherShards();
        // Reset intro state
        currentState = GameState::Intro_Idle;
        currentZoom = 1.0f;
        zoomTimer = 0.0f;
        textRevealTimer = 0.0f;
        winTimer = 0.0f;
        winStartY = 0.0f;
        winCamY = 0.0f;
    };

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    if (fbW > 0 && fbH > 0) restart(fbW, fbH);

    double lastT = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        int nw = 0, nh = 0;
        glfwGetFramebufferSize(window, &nw, &nh);
        if (nw > 0 && nh > 0 && (nw != fbW || nh != fbH)) {
            fbW = nw;
            fbH = nh;
            restart(fbW, fbH);
        }

        double now = glfwGetTime();
        float dt = (float)(now - lastT);
        lastT = now;
        dt = std::min(dt, 0.05f);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, 1);

        bool rPress = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
        if (gameOver && rPress && !rHeld) restart(fbW, fbH);
        rHeld = rPress;
        
        // Intro sequence input handling
        bool spacePress = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                         glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;
        
        // State progression logic
        switch (currentState) {
            case GameState::Intro_Idle:
                if (spacePress && !spaceHeld) {
                    currentState = GameState::Intro_ZoomIn;
                    zoomTimer = 0.0f;
                }
                break;
                
            case GameState::Intro_ZoomIn:
                zoomTimer += dt;
                if (zoomTimer >= 1.5f) {
                    currentZoom = 2.8f;
                    currentState = GameState::Intro_Text;
                    zoomTimer = 0.0f;
                } else {
                    float t = zoomTimer / 1.5f;
                    currentZoom = Lerp(1.0f, 2.8f, t);
                }
                break;
                
            case GameState::Intro_Text:
                // Increment typewriter timer (20 chars + 14 chars at ~10 chars/sec)
                if (textRevealTimer < 35.0f) textRevealTimer += dt * 12.0f;
                if (spacePress && !spaceHeld) {
                    currentState = GameState::Intro_ZoomOut;
                    zoomTimer = 0.0f;
                }
                break;
                
            case GameState::Intro_ZoomOut:
                zoomTimer += dt;
                if (zoomTimer >= 1.0f) {
                    currentZoom = 1.0f;
                    currentState = GameState::Playing;
                    zoomTimer = 0.0f;
                } else {
                    float t = zoomTimer / 1.0f;
                    currentZoom = Lerp(2.8f, 1.0f, t);
                }
                break;
                
            case GameState::Playing:
                // Normal gameplay
                break;
                
            case GameState::Win_Sequence:
                // Frozen ascension - ghost levitates upward
                winTimer += dt;
                ghost.y = winStartY + (winTimer * 45.0f);
                
                // After 2.2s, transition to white void victory
                if (winTimer > 2.2f) {
                    isGameWon = true;
                }
                break;
        }
        spaceHeld = spacePress;

        float camY = ghost.y - fbH * 0.32f;
        if (camY < 0.0f) camY = 0.0f;

        // Check for win condition (20 dots to win for testing)
        // Trigger Win_Sequence cinematic instead of immediate victory
        if (level.dotsCollectedTotal >= 20 && !isGameWon && currentState != GameState::Win_Sequence) {
            currentState = GameState::Win_Sequence;
            winTimer = 0.0f;
            winStartY = ghost.y;
            // Capture frozen camera position
            winCamY = ghost.y - fbH * 0.32f;
            if (winCamY < 0.0f) winCamY = 0.0f;
        }

        // Only update gameplay when in Playing state (skip during Win_Sequence for frozen effect)
        if (currentState == GameState::Playing && !gameOver && fbW > 0 && fbH > 0) {
            std::vector<AABB> colliders = level.PlatformColliders();
            ghost.Update(window, dt, (float)fbW, kGroundTop, colliders.data(), (int)colliders.size());
            level.UpdateMaxHeight(ghost.y);
            walls.Update(dt, (float)fbW, ghost, level);
            if (walls.IsGhostCrushed(ghost)) gameOver = true;
        }

        if (gameOver)
            bloodAmount = std::min(1.0f, bloodAmount + dt * 4.5f);
        else
            bloodAmount = 0.0f;

        glViewport(0, 0, fbW, fbH);
        
        // White screen for game won state
        if (isGameWon) {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        } else {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT);

        if (fbW <= 0 || fbH <= 0) {
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        if (isGameWon) {
            // Game Won state: White void with black feather shards and "Game Ended" text
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, (double)fbW, (double)fbH, 0.0, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            UpdateAndDrawBlackFeatherShards((float)fbW, (float)fbH, dt, (float)now);
            DrawGameEndedText((float)fbW, (float)fbH);
            
            glfwSwapBuffers(window);
            glfwPollEvents();
            continue;
        }

        camY = ghost.y - fbH * 0.32f;
        if (camY < 0.0f) camY = 0.0f;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Explicit state-based camera: intro zoom vs frozen win vs normal gameplay
        if (currentState == GameState::Intro_Idle || 
            currentState == GameState::Intro_ZoomIn || 
            currentState == GameState::Intro_Text || 
            currentState == GameState::Intro_ZoomOut) {
            // Intro: zoom centered on ghost
            double w = fbW / currentZoom;
            double h = fbH / currentZoom;
            double left = ghost.x - w / 2.0;
            double bottom = ghost.y - h / 2.0;
            glOrtho(left, left + w, bottom, bottom + h, -1.0, 1.0);
        } else if (currentState == GameState::Win_Sequence) {
            // Win: frozen camera at capture point
            glOrtho(0.0, (double)fbW, (double)winCamY, (double)winCamY + (double)fbH, -1.0, 1.0);
        } else {
            // Normal gameplay camera
            glOrtho(0.0, (double)fbW, (double)camY, (double)camY + (double)fbH, -1.0, 1.0);
        }
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        level.DrawGround((float)fbW, kGroundTop);
        level.DrawPlatforms(camY, (float)fbH, (float)now);
        DrawPlayerRadialLight(ghost.x, ghost.y, ghost.radius, level.dotsCollectedTotal, (float)now, level.hasFirstBlackDotSpawned);
        DrawTopFixtureArc((float)fbW * 0.5f, camY + (float)fbH, (float)fbW, (float)now, level.dotsCollectedTotal);
        ghost.Draw();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0.0, (double)fbW, 0.0, (double)fbH, -1.0, 1.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        walls.Draw((float)fbW, (float)fbH);

        // White explosion overlay during Win_Sequence
        if (currentState == GameState::Win_Sequence && winTimer > 0.8f) {
            float radius = (winTimer - 0.8f) * 2000.0f;
            // During Win_Sequence, ghost is rendered at screen center (zoom projection)
            // After projection reset, ghost is at center of screen
            float screenGhostX = fbW * 0.5f;
            float screenGhostY = fbH * 0.5f;
            
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(screenGhostX, screenGhostY); // Center
            const int segments = 40;
            for (int i = 0; i <= segments; ++i) {
                float angle = 2.0f * 3.14159265f * i / segments;
                float px = screenGhostX + radius * cosf(angle);
                float py = screenGhostY + radius * sinf(angle);
                glVertex2f(px, py);
            }
            glEnd();
        }

        if (IsBloodTextureLoaded())
            DrawBloodTextureLayer((float)fbW, (float)fbH, bloodAmount);
        else
            DrawBloodOverlay((float)fbW, (float)fbH, bloodAmount, (float)now);
        if (IsBloodTextureLoaded() && bloodAmount > 0.04f)
            DrawBloodOverlay((float)fbW, (float)fbH, bloodAmount * 0.22f, (float)now);

        if (gameOver && bloodAmount > 0.5f)
            DrawGameOverText((float)fbW, (float)fbH);

        // Draw intro text box during Intro_Text state
        if (currentState == GameState::Intro_Text)
            DrawIntroTextBox((float)fbW, (float)fbH, (int)textRevealTimer);

        DrawWhiteDotCountHud((float)fbW, (float)fbH, level.dotsCollectedTotal);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    UnloadBloodTexture();
    glfwTerminate();
    return 0;
}
} // namespace

int main() {
    return RunGhostAscent();
}

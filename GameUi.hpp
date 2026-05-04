#pragma once

// World-space (same ortho as platforms): large soft light on player + top “fixture” arc.
void DrawPlayerRadialLight(float px, float py, float ghostRadius, int dotsGathered, float timeSec, bool hasFirstBlackDotSpawned = false);
void DrawTopFixtureArc(float cx, float topY, float viewW, float timeSec, int dotsGathered);

// Screen-space HUD / overlays (ortho 0..w, 0..h, y-up).
void DrawWhiteDotCountHud(float scrW, float scrH, int whiteDots);
void DrawBloodOverlay(float w, float h, float strength, float timeSec);

// Game Won state rendering
void UpdateAndDrawBlackFeatherShards(float screenW, float screenH, float dt, float timeSec);
void ClearBlackFeatherShards();
void DrawGameEndedText(float screenW, float screenH);

// Game Over state rendering
void DrawGameOverText(float screenW, float screenH);

// Intro sequence rendering
void DrawIntroTextBox(float screenW, float screenH, int charsRevealed);

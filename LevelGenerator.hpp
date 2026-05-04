#pragma once

#include "GhostController.hpp"
#include <vector>

struct DashPlatform {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 28.0f;
    bool hasDot = true;
    bool dotCollected = false;
    bool skipRegistered = false;
};

// Narrow side ledge with a black “relief” dot — collecting opens walls (see WallSystem).
struct SideBlackLedge {
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.0f;
    float h = 24.0f;
    bool isLeft = true;
    bool blackDotCollected = false;
};

// Scale = max(MinScale, StartScale - (maxHeightReached * ScalingFactor))
struct LevelGenerator {
    static constexpr float kStartScale = 220.0f;
    static constexpr float kMinScale = 56.0f;
    static constexpr float kScalingFactor = 0.14f;

    std::vector<DashPlatform> platforms;
    std::vector<SideBlackLedge> sideLedges;
    float maxHeightReached = 0.0f;
    int dotsCollectedTotal = 0;
    unsigned brickSalt = 1337u;
    // Alternates left/right flush spawns for milestone black-dot platforms.
    bool nextBlackPlatformOnLeft = true;
    // Black dot spawn tracking: counter for next spawn, and flag for first spawn
    int blackDotSpawnCounter = 0;
    int blackDotNextSpawnAt = 10; // First spawn at 10, then 2-4 range
    bool hasFirstBlackDotSpawned = false;
    int blackDotsSpawnedCount = 0;

    float PlatformWidthForWorldY(float worldY) const;
    void BuildInitial(int screenWidth, float groundTop);
    void SpawnBlackBorderPlatform(int screenW, float ghostX, float ghostY);
    void UpdateMaxHeight(float ghostY);
    std::vector<AABB> PlatformColliders() const;

    void DrawGround(float worldW, float groundTop) const;
    void DrawPlatforms(float cameraY, float viewH, float timeSec) const;
};

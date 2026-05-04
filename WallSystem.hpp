#pragma once

#include "LevelGenerator.hpp"
#include "GhostController.hpp"

struct WallSystem {
    float leftX = 0.0f;
    float rightX = 0.0f;
    // Must match blade apex inset in Draw() — crush uses blade tips, not bare wall line
    static constexpr float kBladeDepth = 14.0f;

    float baseSpeed = 42.0f;
    float currentSpeed = 42.0f;
    float skipStress = 1.0f;
    float heightStress = 0.0f;

    float bufferTimer = 0.0f;
    static constexpr float kBufferDuration = 2.4f;
    static constexpr float kBufferSpeedMul = 0.38f;

    void Reset(float screenW);
    void OnDotCollected();
    void OnDotSkipped();
    // Ease wall intrusion by 40% (remaining closure = 60% of previous — opens play space).
    void ApplyBlackDotWallRelief(float screenW);

    void Update(float dt, float screenW, GhostController& ghost, LevelGenerator& level);

    bool IsGhostCrushed(const GhostController& ghost) const;
    void Draw(float screenW, float screenH) const;
};

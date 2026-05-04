#pragma once

// AssetExtractor — placeholder for an external image→texture pipeline.
//
// Intended workflow (manual / tooling outside this binary):
// 1. Capture or collect reference photos of old dark-brown brick.
// 2. Use an image model (e.g. Gemini) to propose tiling texture parameters or
//    generate a small PNG; drop it into the project and load with stb_image or WIC.
//
// This build uses procedural bricks (BrickRenderer) so the game runs without
// external assets or network calls.

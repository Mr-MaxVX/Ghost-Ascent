#pragma once

// Loads blood drip PNG (black or green-screen background keyed out). Safe to call once after GL context exists.
bool LoadBloodTexture();
void UnloadBloodTexture();
bool IsBloodTextureLoaded();
// Full-width strip at top of screen (ortho 0..w, 0..h). alpha scales visibility.
void DrawBloodTextureLayer(float screenW, float screenH, float alpha);

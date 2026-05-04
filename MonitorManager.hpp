#pragma once

struct GLFWwindow;

struct MonitorSetup {
    int width = 0;
    int height = 0;
    int posX = 0;
    int posY = 0;
    int refreshRate = 60;
};

MonitorSetup GetPrimaryMonitorSetup();
// Borderless window covering the primary monitor's pixel rectangle (multi-monitor safe position).
GLFWwindow* CreatePrimaryBorderlessWindow(const char* title);

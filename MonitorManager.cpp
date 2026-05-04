#include "MonitorManager.hpp"
#include <GLFW/glfw3.h>

MonitorSetup GetPrimaryMonitorSetup() {
    MonitorSetup s;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (!primary) return s;
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    if (!mode) return s;
    glfwGetMonitorPos(primary, &s.posX, &s.posY);
    s.width = mode->width;
    s.height = mode->height;
    s.refreshRate = mode->refreshRate;
    return s;
}

GLFWwindow* CreatePrimaryBorderlessWindow(const char* title) {
    MonitorSetup s = GetPrimaryMonitorSetup();
    if (s.width <= 0 || s.height <= 0) return nullptr;

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (mode) {
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }
    glfwWindowHint(GLFW_DECORATED, 0);
    glfwWindowHint(GLFW_RESIZABLE, 0);

    GLFWwindow* window = glfwCreateWindow(s.width, s.height, title, nullptr, nullptr);
    if (!window) return nullptr;

    glfwSetWindowPos(window, s.posX, s.posY);
    return window;
}

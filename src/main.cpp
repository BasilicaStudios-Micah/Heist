#include "gamestate.h"
#include <iostream>

// Global pointer for callbacks
static GameState* g_gs = nullptr;

// Track window state
bool isFullscreen = true;
int savedWinX, savedWinY, savedWinW, savedWinH;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // F11 Toggle Logic
        if (key == GLFW_KEY_F11) {
            isFullscreen = !isFullscreen;
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);

            if (isFullscreen) {
                // Save current window position/size before going fullscreen
                glfwGetWindowPos(window, &savedWinX, &savedWinY);
                glfwGetWindowSize(window, &savedWinW, &savedWinH);
                
                // Switch to Windowed Fullscreen (No borders, top-left, monitor res)
                glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
                glfwSetWindowSize(window, mode->width, mode->height);
                glfwSetWindowPos(window, 0, 0);
            } else {
                // Restore to a standard window
                glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_TRUE);
                glfwSetWindowSize(window, 1280, 720); // Default windowed size
                glfwSetWindowPos(window, 100, 100);
            }
            
            // Update the game's internal resolution variables
            int newW, newH;
            glfwGetFramebufferSize(window, &newW, &newH);
            SCREEN_W = newW;
            SCREEN_H = newH;
            glViewport(0, 0, SCREEN_W, SCREEN_H);
        }

        // Keep original ESC behavior
        if (key == GLFW_KEY_ESCAPE && mods == 0) {
            // Optional: You could use this to exit fullscreen first, 
            // but for now let's keep it as your close command.
        }
    }

    if (g_gs) g_gs->onKey(key, action);
}

void cursorCallback(GLFWwindow*, double x, double y) {
    if (g_gs) g_gs->onMouse(x, y);
}

void mouseButtonCallback(GLFWwindow*, int btn, int action, int mods) {
    // Debug print to verify hardware is talking to the software
    // if(action == GLFW_PRESS) printf("Mouse Click: %d\n", btn);
    
    if (g_gs) g_gs->onMouseButton(btn, action);
}

int main() {
    if (!glfwInit()) { return -1; }

    // 1. Get monitor and video mode info
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Initial scale setup
    SCREEN_W = mode->width;
    SCREEN_H = mode->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // 2. Windowed Fullscreen Setup
    // Passing NULL for monitor makes it a "Window", but using monitor resolution
    // and removing decorations (borders) makes it "Windowed Fullscreen".
    // This is MUCH more compatible with mouse input in WSL/Windows.
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "HEIST - Roguelike Bank Robbery", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(window, 0, 0);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // vsync

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { return -1; }

    // 3. Ensure Viewport matches reality
    int fbW, fbH;
    glfwGetFramebufferSize(window, &fbW, &fbH);
    glViewport(0, 0, fbW, fbH);

    // Set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // Ensure the window is focused to capture input
    glfwShowWindow(window);
    glfwFocusWindow(window);

    GameState gs;
    g_gs = &gs;
    gs.init();

    Renderer renderer;
    renderer.init(fbW, fbH);

    double lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        lastTime = now;

        glfwPollEvents();

        renderer.beginFrame();
        gs.update(dt);
        gs.render(renderer);
        renderer.endFrame();

        glfwSwapBuffers(window);
    }

    gs.saveData();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
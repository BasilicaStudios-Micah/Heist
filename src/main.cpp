#include "gamestate.h"

static GameState* g_gs = nullptr;

void keyCallback(GLFWwindow*, int key, int, int action, int){
    if(g_gs) g_gs->onKey(key,action);
}
void cursorCallback(GLFWwindow*, double x, double y){
    if(g_gs) g_gs->onMouse(x,y);
}
void mouseButtonCallback(GLFWwindow*, int btn, int action, int){
    if(g_gs) g_gs->onMouseButton(btn,action);
}

int main(){
    if(!glfwInit()){ return -1; }
    
    // 1. Get monitor and video mode info
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Set runtime screen dimensions to actual monitor resolution FIRST —
    // all game systems (camera, HUD, minimap) read SCREEN_W/SCREEN_H at
    // runtime so they scale correctly to any resolution.
    SCREEN_W = mode->width;
    SCREEN_H = mode->height;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
    // Optional: Use the monitor's refresh rate for VSync consistency
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    // 2. Pass the monitor to enable fullscreen
    // Using mode->width and mode->height ensures it fills the user's screen
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "HEIST - Roguelike Bank Robbery", monitor, nullptr);
    
    if(!window){ glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // vsync

    glewExperimental=GL_TRUE;
    if(glewInit()!=GLEW_OK){ return -1; }

    // 3. Use the actual window dimensions for the viewport
    glViewport(0, 0, mode->width, mode->height);

    // Set callbacks
    glfwSetKeyCallback(window,keyCallback);
    glfwSetCursorPosCallback(window,cursorCallback);
    glfwSetMouseButtonCallback(window,mouseButtonCallback);

    GameState gs;
    g_gs=&gs;
    gs.init();

    Renderer renderer;
    // Update renderer with the actual monitor resolution
    renderer.init(mode->width, mode->height);

    double lastTime=glfwGetTime();

    while(!glfwWindowShouldClose(window)){
        double now=glfwGetTime();
        float dt=(float)(now-lastTime);
        lastTime=now;

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
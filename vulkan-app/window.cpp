#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "window.hpp"

app::window::window()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw_window = glfwCreateWindow(800, 800, "Vulkan", nullptr, nullptr);

    handle = glfwGetWin32Window(glfw_window);
}

app::window::~window()
{
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

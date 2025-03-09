#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include "window.hpp"

app::window::window(void *user_data)
{
    glfwInit();

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* vid_mode = glfwGetVideoMode(monitor);
    m_width = vid_mode->width;
    m_height = vid_mode->height;

    m_width = 800;
    m_height = 800;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfw_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr/*glfwGetPrimaryMonitor()*/, nullptr);

    glfwSetWindowUserPointer(glfw_window, user_data);

    handle = glfwGetWin32Window(glfw_window);
}

app::window::~window()
{
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
}

uint32_t app::window::get_height()
{
    return m_height;
}

uint32_t app::window::get_width()
{
    return m_width;
}
